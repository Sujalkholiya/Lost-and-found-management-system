#include <crow.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "../middleware/authMiddleware.h"

extern sqlite3* db;

static std::string trimString(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static int getCurrentYear()
{
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    return (now ? now->tm_year + 1900 : 2026);
}

// Helper to extract a string column safely
static std::string getColText(sqlite3_stmt* stmt, int colIndex)
{
    const unsigned char* text = sqlite3_column_text(stmt, colIndex);
    return text ? reinterpret_cast<const char*>(text) : "";
}

// POST /api/items/<int>/claims
crow::response createClaim(const crow::request& req, int itemId)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    auto body = crow::json::load(req.body);
    if (!body)
    {
        return crow::response(400, "Invalid JSON data");
    }

    if (!body.has("last_seen_location") ||
        !body.has("unique_identifying_details") ||
        !body.has("item_contents"))
    {
        return crow::response(400, "last_seen_location, unique_identifying_details, and item_contents are required");
    }

    std::string lastSeenLocation = trimString(body["last_seen_location"].s());
    std::string uniqueDetails = trimString(body["unique_identifying_details"].s());
    std::string itemContents = trimString(body["item_contents"].s());
    std::string additionalDetails = body.has("additional_details") ? trimString(body["additional_details"].s()) : "";

    if (lastSeenLocation.empty() || uniqueDetails.empty() || itemContents.empty())
    {
        return crow::response(400, "Verification answers cannot be empty");
    }

    // 1. Verify item exists and check type/status/ownership
    const char* itemQuery = "SELECT type, status, user_id, title FROM items WHERE id = ?;";
    sqlite3_stmt* itemStmt;
    if (sqlite3_prepare_v2(db, itemQuery, -1, &itemStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error");
    }
    sqlite3_bind_int(itemStmt, 1, itemId);

    if (sqlite3_step(itemStmt) != SQLITE_ROW)
    {
        sqlite3_finalize(itemStmt);
        return crow::response(404, "Item not found");
    }

    std::string itemType = getColText(itemStmt, 0);
    std::string itemStatus = getColText(itemStmt, 1);
    int itemOwnerId = sqlite3_column_int(itemStmt, 2);
    std::string itemTitle = getColText(itemStmt, 3);
    sqlite3_finalize(itemStmt);

    // Business rule: Can only claim found items
    std::string itemTypeLower = itemType;
    std::transform(itemTypeLower.begin(), itemTypeLower.end(), itemTypeLower.begin(), ::tolower);
    if (itemTypeLower != "found")
    {
        return crow::response(400, "Only found items can be claimed");
    }

    // Business rule: Cannot claim already returned item
    std::string itemStatusLower = itemStatus;
    std::transform(itemStatusLower.begin(), itemStatusLower.end(), itemStatusLower.begin(), ::tolower);
    if (itemStatusLower == "returned")
    {
        return crow::response(400, "This item has already been returned to its owner");
    }

    // Business rule: Cannot claim your own reported item
    if (itemOwnerId == user.id)
    {
        return crow::response(400, "You cannot claim an item you reported");
    }

    // 2. Check for duplicate active claim by this user on this item
    const char* activeClaimQuery = "SELECT id FROM claims WHERE item_id = ? AND user_id = ? AND status IN ('pending', 'approved');";
    sqlite3_stmt* dupStmt;
    if (sqlite3_prepare_v2(db, activeClaimQuery, -1, &dupStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error");
    }
    sqlite3_bind_int(dupStmt, 1, itemId);
    sqlite3_bind_int(dupStmt, 2, user.id);

    if (sqlite3_step(dupStmt) == SQLITE_ROW)
    {
        sqlite3_finalize(dupStmt);
        return crow::response(400, "You already have an active claim for this item");
    }
    sqlite3_finalize(dupStmt);

    // 3. Generate claim reference: CLM-YYYY-XXXXX
    int nextSeq = 1;
    const char* seqQuery = "SELECT COALESCE(MAX(id), 0) + 1 FROM claims;";
    sqlite3_stmt* seqStmt;
    if (sqlite3_prepare_v2(db, seqQuery, -1, &seqStmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(seqStmt) == SQLITE_ROW)
        {
            nextSeq = sqlite3_column_int(seqStmt, 0);
        }
        sqlite3_finalize(seqStmt);
    }

    int year = getCurrentYear();
    std::ostringstream refStream;
    refStream << "CLM-" << year << "-" << std::setw(5) << std::setfill('0') << (100 + nextSeq);
    std::string claimReference = refStream.str();

    // 4. Insert claim
    const char* insertQuery = R"(
        INSERT INTO claims (
            claim_reference, item_id, user_id, status,
            last_seen_location, unique_identifying_details, item_contents,
            additional_details, pickup_location, created_at
        ) VALUES (?, ?, ?, 'pending', ?, ?, ?, ?, 'Admin Block - Room 102', datetime('now', 'localtime'));
    )";

    sqlite3_stmt* insertStmt;
    if (sqlite3_prepare_v2(db, insertQuery, -1, &insertStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error preparing claim insertion");
    }

    sqlite3_bind_text(insertStmt, 1, claimReference.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertStmt, 2, itemId);
    sqlite3_bind_int(insertStmt, 3, user.id);
    sqlite3_bind_text(insertStmt, 4, lastSeenLocation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 5, uniqueDetails.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 6, itemContents.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 7, additionalDetails.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(insertStmt) != SQLITE_DONE)
    {
        sqlite3_finalize(insertStmt);
        return crow::response(500, "Failed to submit claim");
    }
    sqlite3_finalize(insertStmt);

    long long newClaimId = sqlite3_last_insert_rowid(db);

    crow::json::wvalue resp;
    resp["message"] = "Claim submitted successfully";
    resp["claim"]["id"] = newClaimId;
    resp["claim"]["claim_reference"] = claimReference;
    resp["claim"]["item_id"] = itemId;
    resp["claim"]["item_title"] = itemTitle;
    resp["claim"]["status"] = "pending";
    resp["claim"]["pickup_location"] = "Admin Block - Room 102";

    return crow::response(201, resp);
}

// GET /api/users/me/claims
crow::response getUserClaims(const crow::request& req)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    const char* query = R"(
        SELECT
            c.id, c.claim_reference, c.item_id, c.user_id, c.status,
            c.last_seen_location, c.unique_identifying_details, c.item_contents,
            c.additional_details, c.rejection_reason, c.pickup_location,
            c.reviewed_by, c.handover_admin_id, c.created_at, c.reviewed_at,
            c.approved_at, c.handed_over_at,
            i.title, i.description, i.location, i.type, i.image_path, i.status AS item_status
        FROM claims c
        JOIN items i ON c.item_id = i.id
        WHERE c.user_id = ?
        ORDER BY c.id DESC;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database query failed");
    }
    sqlite3_bind_int(stmt, 1, user.id);

    crow::json::wvalue resp;
    int index = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        resp["claims"][index]["id"] = sqlite3_column_int(stmt, 0);
        resp["claims"][index]["claim_reference"] = getColText(stmt, 1);
        resp["claims"][index]["item_id"] = sqlite3_column_int(stmt, 2);
        resp["claims"][index]["user_id"] = sqlite3_column_int(stmt, 3);
        resp["claims"][index]["status"] = getColText(stmt, 4);
        resp["claims"][index]["last_seen_location"] = getColText(stmt, 5);
        resp["claims"][index]["unique_identifying_details"] = getColText(stmt, 6);
        resp["claims"][index]["item_contents"] = getColText(stmt, 7);
        resp["claims"][index]["additional_details"] = getColText(stmt, 8);
        resp["claims"][index]["rejection_reason"] = getColText(stmt, 9);
        resp["claims"][index]["pickup_location"] = getColText(stmt, 10);
        resp["claims"][index]["reviewed_by"] = sqlite3_column_int(stmt, 11);
        resp["claims"][index]["handover_admin_id"] = sqlite3_column_int(stmt, 12);
        resp["claims"][index]["created_at"] = getColText(stmt, 13);
        resp["claims"][index]["reviewed_at"] = getColText(stmt, 14);
        resp["claims"][index]["approved_at"] = getColText(stmt, 15);
        resp["claims"][index]["handed_over_at"] = getColText(stmt, 16);

        // Joined item fields
        resp["claims"][index]["item_title"] = getColText(stmt, 17);
        resp["claims"][index]["item_description"] = getColText(stmt, 18);
        resp["claims"][index]["item_location"] = getColText(stmt, 19);
        resp["claims"][index]["item_type"] = getColText(stmt, 20);
        resp["claims"][index]["item_image_path"] = getColText(stmt, 21);
        resp["claims"][index]["item_status"] = getColText(stmt, 22);

        index++;
    }
    sqlite3_finalize(stmt);

    resp["count"] = index;
    return crow::response(200, resp);
}

// GET /api/items/<int>/my-claim
crow::response getMyClaimForItem(const crow::request& req, int itemId)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    const char* query = R"(
        SELECT
            c.id, c.claim_reference, c.item_id, c.user_id, c.status,
            c.last_seen_location, c.unique_identifying_details, c.item_contents,
            c.additional_details, c.rejection_reason, c.pickup_location,
            c.created_at, c.approved_at, c.handed_over_at
        FROM claims c
        WHERE c.item_id = ? AND c.user_id = ?
        ORDER BY c.id DESC
        LIMIT 1;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database query failed");
    }
    sqlite3_bind_int(stmt, 1, itemId);
    sqlite3_bind_int(stmt, 2, user.id);

    crow::json::wvalue resp;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        resp["has_claim"] = true;
        resp["claim"]["id"] = sqlite3_column_int(stmt, 0);
        resp["claim"]["claim_reference"] = getColText(stmt, 1);
        resp["claim"]["item_id"] = sqlite3_column_int(stmt, 2);
        resp["claim"]["user_id"] = sqlite3_column_int(stmt, 3);
        resp["claim"]["status"] = getColText(stmt, 4);
        resp["claim"]["last_seen_location"] = getColText(stmt, 5);
        resp["claim"]["unique_identifying_details"] = getColText(stmt, 6);
        resp["claim"]["item_contents"] = getColText(stmt, 7);
        resp["claim"]["additional_details"] = getColText(stmt, 8);
        resp["claim"]["rejection_reason"] = getColText(stmt, 9);
        resp["claim"]["pickup_location"] = getColText(stmt, 10);
        resp["claim"]["created_at"] = getColText(stmt, 11);
        resp["claim"]["approved_at"] = getColText(stmt, 12);
        resp["claim"]["handed_over_at"] = getColText(stmt, 13);
    }
    else
    {
        resp["has_claim"] = false;
    }
    sqlite3_finalize(stmt);

    return crow::response(200, resp);
}

// GET /api/claims/<int>
crow::response getClaimById(const crow::request& req, int id)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    const char* query = R"(
        SELECT
            c.id, c.claim_reference, c.item_id, c.user_id, c.status,
            c.last_seen_location, c.unique_identifying_details, c.item_contents,
            c.additional_details, c.rejection_reason, c.pickup_location,
            c.reviewed_by, c.handover_admin_id, c.created_at, c.reviewed_at,
            c.approved_at, c.handed_over_at,
            i.title, i.description, i.location, i.type, i.image_path, i.status AS item_status,
            u.name AS claimant_name, u.email AS claimant_email
        FROM claims c
        JOIN items i ON c.item_id = i.id
        JOIN users u ON c.user_id = u.id
        WHERE c.id = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database query failed");
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        return crow::response(404, "Claim not found");
    }

    int claimUserId = sqlite3_column_int(stmt, 3);
    if (user.role != "admin" && user.id != claimUserId)
    {
        sqlite3_finalize(stmt);
        return crow::response(403, "Access denied");
    }

    crow::json::wvalue resp;
    resp["id"] = sqlite3_column_int(stmt, 0);
    resp["claim_reference"] = getColText(stmt, 1);
    resp["item_id"] = sqlite3_column_int(stmt, 2);
    resp["user_id"] = claimUserId;
    resp["status"] = getColText(stmt, 4);
    resp["last_seen_location"] = getColText(stmt, 5);
    resp["unique_identifying_details"] = getColText(stmt, 6);
    resp["item_contents"] = getColText(stmt, 7);
    resp["additional_details"] = getColText(stmt, 8);
    resp["rejection_reason"] = getColText(stmt, 9);
    resp["pickup_location"] = getColText(stmt, 10);
    resp["reviewed_by"] = sqlite3_column_int(stmt, 11);
    resp["handover_admin_id"] = sqlite3_column_int(stmt, 12);
    resp["created_at"] = getColText(stmt, 13);
    resp["reviewed_at"] = getColText(stmt, 14);
    resp["approved_at"] = getColText(stmt, 15);
    resp["handed_over_at"] = getColText(stmt, 16);

    resp["item_title"] = getColText(stmt, 17);
    resp["item_description"] = getColText(stmt, 18);
    resp["item_location"] = getColText(stmt, 19);
    resp["item_type"] = getColText(stmt, 20);
    resp["item_image_path"] = getColText(stmt, 21);
    resp["item_status"] = getColText(stmt, 22);

    resp["claimant_name"] = getColText(stmt, 23);
    resp["claimant_email"] = getColText(stmt, 24);

    sqlite3_finalize(stmt);
    return crow::response(200, resp);
}

// GET /api/admin/claims
crow::response getAdminClaims(const crow::request& req)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    if (user.role != "admin")
    {
        return crow::response(403, "Admin access required");
    }

    const char* statusParam = req.url_params.get("status");
    std::string filterStatus = statusParam ? statusParam : "";

    std::string query = R"(
        SELECT
            c.id, c.claim_reference, c.item_id, c.user_id, c.status,
            c.last_seen_location, c.unique_identifying_details, c.item_contents,
            c.additional_details, c.rejection_reason, c.pickup_location,
            c.reviewed_by, c.handover_admin_id, c.created_at, c.reviewed_at,
            c.approved_at, c.handed_over_at,
            i.title, i.description, i.location, i.type, i.image_path, i.status AS item_status,
            u.name AS claimant_name, u.email AS claimant_email
        FROM claims c
        JOIN items i ON c.item_id = i.id
        JOIN users u ON c.user_id = u.id
    )";

    if (!filterStatus.empty() && filterStatus != "all")
    {
        query += " WHERE c.status = ? ";
    }

    query += R"(
        ORDER BY
            CASE
                WHEN c.status = 'pending' THEN 1
                WHEN c.status = 'approved' THEN 2
                ELSE 3
            END,
            c.id DESC;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database query failed");
    }

    if (!filterStatus.empty() && filterStatus != "all")
    {
        sqlite3_bind_text(stmt, 1, filterStatus.c_str(), -1, SQLITE_TRANSIENT);
    }

    crow::json::wvalue resp;
    int index = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        resp["claims"][index]["id"] = sqlite3_column_int(stmt, 0);
        resp["claims"][index]["claim_reference"] = getColText(stmt, 1);
        resp["claims"][index]["item_id"] = sqlite3_column_int(stmt, 2);
        resp["claims"][index]["user_id"] = sqlite3_column_int(stmt, 3);
        resp["claims"][index]["status"] = getColText(stmt, 4);
        resp["claims"][index]["last_seen_location"] = getColText(stmt, 5);
        resp["claims"][index]["unique_identifying_details"] = getColText(stmt, 6);
        resp["claims"][index]["item_contents"] = getColText(stmt, 7);
        resp["claims"][index]["additional_details"] = getColText(stmt, 8);
        resp["claims"][index]["rejection_reason"] = getColText(stmt, 9);
        resp["claims"][index]["pickup_location"] = getColText(stmt, 10);
        resp["claims"][index]["reviewed_by"] = sqlite3_column_int(stmt, 11);
        resp["claims"][index]["handover_admin_id"] = sqlite3_column_int(stmt, 12);
        resp["claims"][index]["created_at"] = getColText(stmt, 13);
        resp["claims"][index]["reviewed_at"] = getColText(stmt, 14);
        resp["claims"][index]["approved_at"] = getColText(stmt, 15);
        resp["claims"][index]["handed_over_at"] = getColText(stmt, 16);

        resp["claims"][index]["item_title"] = getColText(stmt, 17);
        resp["claims"][index]["item_description"] = getColText(stmt, 18);
        resp["claims"][index]["item_location"] = getColText(stmt, 19);
        resp["claims"][index]["item_type"] = getColText(stmt, 20);
        resp["claims"][index]["item_image_path"] = getColText(stmt, 21);
        resp["claims"][index]["item_status"] = getColText(stmt, 22);

        resp["claims"][index]["claimant_name"] = getColText(stmt, 23);
        resp["claims"][index]["claimant_email"] = getColText(stmt, 24);

        index++;
    }
    sqlite3_finalize(stmt);
    resp["count"] = index;

    // Calculate claim stats/KPIs
    int totalCount = 0;
    int pendingCount = 0;
    int approvedCount = 0;
    int handedOverCount = 0;
    int rejectedCount = 0;

    const char* statsQuery = R"(
        SELECT
            COUNT(*),
            COALESCE(SUM(CASE WHEN status = 'pending' THEN 1 ELSE 0 END), 0),
            COALESCE(SUM(CASE WHEN status = 'approved' THEN 1 ELSE 0 END), 0),
            COALESCE(SUM(CASE WHEN status = 'handed_over' THEN 1 ELSE 0 END), 0),
            COALESCE(SUM(CASE WHEN status = 'rejected' THEN 1 ELSE 0 END), 0)
        FROM claims;
    )";

    sqlite3_stmt* statsStmt;
    if (sqlite3_prepare_v2(db, statsQuery, -1, &statsStmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(statsStmt) == SQLITE_ROW)
        {
            totalCount = sqlite3_column_int(statsStmt, 0);
            pendingCount = sqlite3_column_int(statsStmt, 1);
            approvedCount = sqlite3_column_int(statsStmt, 2);
            handedOverCount = sqlite3_column_int(statsStmt, 3);
            rejectedCount = sqlite3_column_int(statsStmt, 4);
        }
        sqlite3_finalize(statsStmt);
    }

    resp["stats"]["total"] = totalCount;
    resp["stats"]["pending"] = pendingCount;
    resp["stats"]["approved"] = approvedCount;
    resp["stats"]["handed_over"] = handedOverCount;
    resp["stats"]["rejected"] = rejectedCount;

    return crow::response(200, resp);
}

// PUT /api/admin/claims/<int>/approve
crow::response approveClaim(const crow::request& req, int id)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    if (user.role != "admin")
    {
        return crow::response(403, "Admin access required");
    }

    // Check claim status
    const char* checkQuery = "SELECT status FROM claims WHERE id = ?;";
    sqlite3_stmt* checkStmt;
    if (sqlite3_prepare_v2(db, checkQuery, -1, &checkStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error");
    }
    sqlite3_bind_int(checkStmt, 1, id);

    if (sqlite3_step(checkStmt) != SQLITE_ROW)
    {
        sqlite3_finalize(checkStmt);
        return crow::response(404, "Claim not found");
    }

    std::string currentStatus = getColText(checkStmt, 0);
    sqlite3_finalize(checkStmt);

    if (currentStatus == "handed_over")
    {
        return crow::response(400, "Claim has already been handed over");
    }

    std::string pickupLocation = "Admin Block - Room 102";
    auto body = crow::json::load(req.body);
    if (body && body.has("pickup_location"))
    {
        std::string loc = trimString(body["pickup_location"].s());
        if (!loc.empty())
        {
            pickupLocation = loc;
        }
    }

    const char* updateQuery = R"(
        UPDATE claims
        SET status = 'approved',
            pickup_location = ?,
            reviewed_by = ?,
            reviewed_at = datetime('now', 'localtime'),
            approved_at = datetime('now', 'localtime')
        WHERE id = ?;
    )";

    sqlite3_stmt* updateStmt;
    if (sqlite3_prepare_v2(db, updateQuery, -1, &updateStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error preparing approval");
    }
    sqlite3_bind_text(updateStmt, 1, pickupLocation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(updateStmt, 2, user.id);
    sqlite3_bind_int(updateStmt, 3, id);

    if (sqlite3_step(updateStmt) != SQLITE_DONE)
    {
        sqlite3_finalize(updateStmt);
        return crow::response(500, "Failed to approve claim");
    }
    sqlite3_finalize(updateStmt);

    crow::json::wvalue resp;
    resp["message"] = "Claim approved successfully. Claimant may proceed for physical pickup.";
    resp["claim_id"] = id;
    resp["status"] = "approved";
    resp["pickup_location"] = pickupLocation;

    return crow::response(200, resp);
}

// PUT /api/admin/claims/<int>/reject
crow::response rejectClaim(const crow::request& req, int id)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    if (user.role != "admin")
    {
        return crow::response(403, "Admin access required");
    }

    auto body = crow::json::load(req.body);
    if (!body || !body.has("rejection_reason"))
    {
        return crow::response(400, "rejection_reason is required");
    }

    std::string reason = trimString(body["rejection_reason"].s());
    if (reason.empty())
    {
        return crow::response(400, "rejection_reason cannot be empty");
    }

    // Check claim status
    const char* checkQuery = "SELECT status FROM claims WHERE id = ?;";
    sqlite3_stmt* checkStmt;
    if (sqlite3_prepare_v2(db, checkQuery, -1, &checkStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error");
    }
    sqlite3_bind_int(checkStmt, 1, id);

    if (sqlite3_step(checkStmt) != SQLITE_ROW)
    {
        sqlite3_finalize(checkStmt);
        return crow::response(404, "Claim not found");
    }

    std::string currentStatus = getColText(checkStmt, 0);
    sqlite3_finalize(checkStmt);

    if (currentStatus == "handed_over")
    {
        return crow::response(400, "Cannot reject a claim that has already been handed over");
    }

    const char* updateQuery = R"(
        UPDATE claims
        SET status = 'rejected',
            rejection_reason = ?,
            reviewed_by = ?,
            reviewed_at = datetime('now', 'localtime')
        WHERE id = ?;
    )";

    sqlite3_stmt* updateStmt;
    if (sqlite3_prepare_v2(db, updateQuery, -1, &updateStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error preparing rejection");
    }
    sqlite3_bind_text(updateStmt, 1, reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(updateStmt, 2, user.id);
    sqlite3_bind_int(updateStmt, 3, id);

    if (sqlite3_step(updateStmt) != SQLITE_DONE)
    {
        sqlite3_finalize(updateStmt);
        return crow::response(500, "Failed to reject claim");
    }
    sqlite3_finalize(updateStmt);

    crow::json::wvalue resp;
    resp["message"] = "Claim rejected";
    resp["claim_id"] = id;
    resp["status"] = "rejected";
    resp["rejection_reason"] = reason;

    return crow::response(200, resp);
}

// PUT /api/admin/claims/<int>/handover
crow::response handoverClaim(const crow::request& req, int id)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    if (user.role != "admin")
    {
        return crow::response(403, "Admin access required");
    }

    // Check claim status and get item_id
    const char* checkQuery = "SELECT status, item_id FROM claims WHERE id = ?;";
    sqlite3_stmt* checkStmt;
    if (sqlite3_prepare_v2(db, checkQuery, -1, &checkStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error");
    }
    sqlite3_bind_int(checkStmt, 1, id);

    if (sqlite3_step(checkStmt) != SQLITE_ROW)
    {
        sqlite3_finalize(checkStmt);
        return crow::response(404, "Claim not found");
    }

    std::string currentStatus = getColText(checkStmt, 0);
    int itemId = sqlite3_column_int(checkStmt, 1);
    sqlite3_finalize(checkStmt);

    if (currentStatus == "handed_over")
    {
        return crow::response(400, "Claim has already been handed over");
    }

    if (currentStatus != "approved")
    {
        return crow::response(400, "Claim must be in 'approved' status before physical handover");
    }

    // 1. Update claim to handed_over
    const char* claimUpdateQuery = R"(
        UPDATE claims
        SET status = 'handed_over',
            handover_admin_id = ?,
            handed_over_at = datetime('now', 'localtime')
        WHERE id = ?;
    )";

    sqlite3_stmt* claimStmt;
    if (sqlite3_prepare_v2(db, claimUpdateQuery, -1, &claimStmt, nullptr) != SQLITE_OK)
    {
        return crow::response(500, "Database error preparing claim handover");
    }
    sqlite3_bind_int(claimStmt, 1, user.id);
    sqlite3_bind_int(claimStmt, 2, id);

    if (sqlite3_step(claimStmt) != SQLITE_DONE)
    {
        sqlite3_finalize(claimStmt);
        return crow::response(500, "Failed to update claim handover status");
    }
    sqlite3_finalize(claimStmt);

    // 2. Mark item as returned
    const char* itemUpdateQuery = "UPDATE items SET status = 'returned' WHERE id = ?;";
    sqlite3_stmt* itemStmt;
    if (sqlite3_prepare_v2(db, itemUpdateQuery, -1, &itemStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(itemStmt, 1, itemId);
        sqlite3_step(itemStmt);
        sqlite3_finalize(itemStmt);
    }

    // 3. Auto-reject any remaining pending claims on this same item
    const char* rejectOtherQuery = R"(
        UPDATE claims
        SET status = 'rejected',
            rejection_reason = 'Item has been handed over to verified owner.',
            reviewed_by = ?,
            reviewed_at = datetime('now', 'localtime')
        WHERE item_id = ? AND id != ? AND status = 'pending';
    )";
    sqlite3_stmt* otherStmt;
    if (sqlite3_prepare_v2(db, rejectOtherQuery, -1, &otherStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(otherStmt, 1, user.id);
        sqlite3_bind_int(otherStmt, 2, itemId);
        sqlite3_bind_int(otherStmt, 3, id);
        sqlite3_step(otherStmt);
        sqlite3_finalize(otherStmt);
    }

    crow::json::wvalue resp;
    resp["message"] = "Item handed over successfully. Item status marked as returned.";
    resp["claim_id"] = id;
    resp["item_id"] = itemId;
    resp["status"] = "handed_over";

    return crow::response(200, resp);
}

