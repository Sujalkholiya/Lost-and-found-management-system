#include <crow.h>
#include <crow/multipart.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <atomic>
#include <algorithm>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include "../middleware/authMiddleware.h"

extern sqlite3* db;

static std::string getFileExtension(const std::string& filename)
{
    size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext;
}

static bool isValidImageExtension(const std::string& ext)
{
    return (ext == ".jpg" || ext == ".jpeg" || ext == ".png");
}

crow::response createItem(const crow::request& req)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        return crow::response(401, "Unauthorized");
    }

    int userId = user.id;
    std::string title;
    std::string description;
    std::string type;
    std::string location;
    std::string imagePath = "";

    std::string contentType = req.get_header_value("Content-Type");
    bool isMultipart = (contentType.find("multipart/form-data") != std::string::npos);

    if (isMultipart)
    {
        crow::multipart::message msg(req);

        auto titlePart = msg.get_part_by_name("title");
        auto descPart = msg.get_part_by_name("description");
        auto typePart = msg.get_part_by_name("type");
        auto locPart = msg.get_part_by_name("location");

        title = titlePart.body;
        description = descPart.body;
        type = typePart.body;
        location = locPart.body;

        if (title.empty() || description.empty() || type.empty() || location.empty())
        {
            return crow::response(
                400,
                "title, description, type, location and user_id are required"
            );
        }

        auto imagePart = msg.get_part_by_name("image");
        if (!imagePart.body.empty())
        {
            // Enforce 5 MB maximum size limit
            constexpr size_t MAX_IMAGE_SIZE = 5 * 1024 * 1024;
            if (imagePart.body.size() > MAX_IMAGE_SIZE)
            {
                return crow::response(400, "Image exceeds maximum allowed size of 5 MB");
            }

            auto dispHeader = imagePart.get_header_object("Content-Disposition");
            std::string originalFilename;
            auto it = dispHeader.params.find("filename");
            if (it != dispHeader.params.end())
            {
                originalFilename = it->second;
            }

            std::string ext = getFileExtension(originalFilename);
            if (!isValidImageExtension(ext))
            {
                return crow::response(400, "Invalid image format. Only JPG, JPEG, and PNG are allowed.");
            }

            static std::atomic<uint64_t> counter{0};
            auto now = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
            uint64_t count = ++counter;

            std::string uniqueFilename = "item_" + std::to_string(ms) + "_" + std::to_string(count) + ext;

#ifdef _WIN32
            _mkdir("uploads");
#else
            mkdir("uploads", 0755);
#endif

            std::string fullPath = "uploads/" + uniqueFilename;
            std::ofstream outFile(fullPath, std::ios::binary);
            if (!outFile.is_open())
            {
                return crow::response(500, "Failed to save uploaded image");
            }
            outFile.write(imagePart.body.data(), static_cast<std::streamsize>(imagePart.body.size()));
            outFile.close();

            imagePath = uniqueFilename;
        }
    }
    else
    {
        auto body = crow::json::load(req.body);
        if (!body)
        {
            return crow::response(400, "Invalid JSON data");
        }

        if (!body.has("title") ||
            !body.has("description") ||
            !body.has("type") ||
            !body.has("location"))
        {
            return crow::response(
                400,
                "title, description, type, location and user_id are required"
            );
        }

        title = body["title"].s();
        description = body["description"].s();
        type = body["type"].s();
        location = body["location"].s();

        if (body.has("image_path"))
        {
            imagePath = body["image_path"].s();
        }
    }

    // Only allow lost or found
    if (type != "lost" && type != "found")
    {
        return crow::response(
            400,
            "Type must be either lost or found"
        );
    }

    const char* query =
        "INSERT INTO items "
        "(title, description, type, location, user_id, image_path) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* statement;
    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return crow::response(
            500,
            "Database query preparation failed"
        );
    }

    sqlite3_bind_text(
        statement,
        1,
        title.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        2,
        description.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        3,
        type.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        4,
        location.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_int(
        statement,
        5,
        userId
    );

    sqlite3_bind_text(
        statement,
        6,
        imagePath.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        return crow::response(
            500,
            "Failed to create item"
        );
    }

    int itemId = static_cast<int>(
        sqlite3_last_insert_rowid(db)
    );

    crow::json::wvalue response;
    response["message"] = "Item created successfully";
    response["item"]["id"] = itemId;
    response["item"]["title"] = title;
    response["item"]["description"] = description;
    response["item"]["type"] = type;
    response["item"]["location"] = location;
    response["item"]["user_id"] = userId;
    response["item"]["image_path"] = imagePath;

    return crow::response(201, response);
}

crow::response getAllItems(const crow::request& req)
{
    const char* query =
        "SELECT id, title, description, type, location, user_id, image_path "
        "FROM items "
        "ORDER BY id DESC;";

    sqlite3_stmt* statement;
    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return crow::response(
            500,
            "Database query preparation failed"
        );
    }

    crow::json::wvalue response;
    int index = 0;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        response["items"][index]["id"] =
            sqlite3_column_int(statement, 0);

        response["items"][index]["title"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 1)
            );

        response["items"][index]["description"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 2)
            );

        response["items"][index]["type"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 3)
            );

        response["items"][index]["location"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 4)
            );

        response["items"][index]["user_id"] =
            sqlite3_column_int(statement, 5);

        const char* imgPath = reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 6)
        );
        response["items"][index]["image_path"] = imgPath ? imgPath : "";

        index++;
    }

    sqlite3_finalize(statement);
    response["count"] = index;
    return crow::response(200, response);
}

crow::response getItemById(const crow::request& req, int id)
{
    const char* query =
        "SELECT id, title, description, type, location, user_id, image_path "
        "FROM items "
        "WHERE id = ?;";

    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return crow::response(
            500,
            "Database query preparation failed"
        );
    }

    sqlite3_bind_int(
        statement,
        1,
        id
    );

    result = sqlite3_step(statement);

    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);

        return crow::response(
            404,
            "Item not found"
        );
    }

    crow::json::wvalue response;

    response["item"]["id"] =
        sqlite3_column_int(statement, 0);

    response["item"]["title"] =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 1)
        );

    response["item"]["description"] =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 2)
        );

    response["item"]["type"] =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 3)
        );

    response["item"]["location"] =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 4)
        );

    response["item"]["user_id"] =
        sqlite3_column_int(statement, 5);

    const char* imgPath = reinterpret_cast<const char*>(
        sqlite3_column_text(statement, 6)
    );
    response["item"]["image_path"] = imgPath ? imgPath : "";

    sqlite3_finalize(statement);

    return crow::response(200, response);
}

crow::response updateItem(
    const crow::request& req,
    int id
)
{
    auto body = crow::json::load(req.body);

    if (!body)
    {
        return crow::response(
            400,
            "Invalid JSON data"
        );
    }


    if (!body.has("title") ||
        !body.has("description") ||
        !body.has("type") ||
        !body.has("location"))
    {
        return crow::response(
            400,
            "title, description, type and location are required"
        );
    }


    std::string title = body["title"].s();
    std::string description = body["description"].s();
    std::string type = body["type"].s();
    std::string location = body["location"].s();


    // Only allow lost or found
    if (type != "lost" && type != "found")
    {
        return crow::response(
            400,
            "Type must be either lost or found"
        );
    }


    const char* query =
        "UPDATE items "
        "SET title = ?, "
        "description = ?, "
        "type = ?, "
        "location = ? "
        "WHERE id = ?;";


    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );


    if (result != SQLITE_OK)
    {
        return crow::response(
            500,
            "Database query preparation failed"
        );
    }


    sqlite3_bind_text(
        statement,
        1,
        title.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        2,
        description.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        3,
        type.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        4,
        location.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_int(
        statement,
        5,
        id
    );


    result = sqlite3_step(statement);

    sqlite3_finalize(statement);


    if (result != SQLITE_DONE)
    {
        return crow::response(
            500,
            "Failed to update item"
        );
    }


    if (sqlite3_changes(db) == 0)
    {
        return crow::response(
            404,
            "Item not found"
        );
    }


    crow::json::wvalue response;

    response["message"] = "Item updated successfully";

    response["item"]["id"] = id;
    response["item"]["title"] = title;
    response["item"]["description"] = description;
    response["item"]["type"] = type;
    response["item"]["location"] = location;


    return crow::response(200, response);
}

crow::response deleteItem(
    const crow::request& req,
    int id
)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        crow::json::wvalue err;
        err["message"] = "Unauthorized";
        return crow::response(401, err);
    }

    // 1. Look up item by id to verify existence and check owner
    const char* selectQuery =
        "SELECT user_id, image_path FROM items WHERE id = ?;";

    sqlite3_stmt* selectStmt;
    int prepResult = sqlite3_prepare_v2(
        db,
        selectQuery,
        -1,
        &selectStmt,
        nullptr
    );

    if (prepResult != SQLITE_OK)
    {
        crow::json::wvalue err;
        err["message"] = "Database query preparation failed";
        return crow::response(500, err);
    }

    sqlite3_bind_int(selectStmt, 1, id);

    int stepResult = sqlite3_step(selectStmt);
    if (stepResult != SQLITE_ROW)
    {
        sqlite3_finalize(selectStmt);
        crow::json::wvalue err;
        err["message"] = "Item not found";
        return crow::response(404, err);
    }

    int itemUserId = sqlite3_column_int(selectStmt, 0);
    const char* imgPathCol = reinterpret_cast<const char*>(sqlite3_column_text(selectStmt, 1));
    std::string itemImagePath = imgPathCol ? imgPathCol : "";
    sqlite3_finalize(selectStmt);

    // 2. Check authorization: owner or admin
    if (user.role != "admin" && user.id != itemUserId)
    {
        crow::json::wvalue err;
        err["message"] = "You can only delete your own reported items";
        return crow::response(403, err);
    }

    // 3. Delete the item row
    const char* deleteQuery =
        "DELETE FROM items WHERE id = ?;";

    sqlite3_stmt* deleteStmt;
    prepResult = sqlite3_prepare_v2(
        db,
        deleteQuery,
        -1,
        &deleteStmt,
        nullptr
    );

    if (prepResult != SQLITE_OK)
    {
        crow::json::wvalue err;
        err["message"] = "Database query preparation failed";
        return crow::response(500, err);
    }

    sqlite3_bind_int(deleteStmt, 1, id);
    stepResult = sqlite3_step(deleteStmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(deleteStmt);

    if (stepResult != SQLITE_DONE || changes == 0)
    {
        crow::json::wvalue err;
        err["message"] = "Failed to delete item";
        return crow::response(500, err);
    }

    // 4. Delete associated image file from uploads/ folder if present
    if (!itemImagePath.empty())
    {
        // Sanitize check
        if (itemImagePath.find("..") == std::string::npos &&
            itemImagePath.find('/') == std::string::npos &&
            itemImagePath.find('\\') == std::string::npos)
        {
            std::string filePath = "uploads/" + itemImagePath;
            std::remove(filePath.c_str());
        }
    }

    crow::json::wvalue response;
    response["message"] = "Item deleted successfully";
    return crow::response(200, response);
}

crow::response getUserItems(const crow::request& req)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        crow::json::wvalue err;
        err["message"] = "Unauthorized";
        return crow::response(401, err);
    }

    const char* query =
        "SELECT id, title, description, type, location, status, user_id, created_at, image_path "
        "FROM items "
        "WHERE user_id = ? "
        "ORDER BY id DESC;";

    sqlite3_stmt* statement;
    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        crow::json::wvalue err;
        err["message"] = "Database query preparation failed";
        return crow::response(500, err);
    }

    sqlite3_bind_int(statement, 1, user.id);

    crow::json::wvalue response;
    int index = 0;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        response["items"][index]["id"] = sqlite3_column_int(statement, 0);
        response["items"][index]["title"] = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        response["items"][index]["description"] = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
        response["items"][index]["type"] = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        response["items"][index]["location"] = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
        const char* st = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        response["items"][index]["status"] = st ? st : "active";
        response["items"][index]["user_id"] = sqlite3_column_int(statement, 6);
        const char* ca = reinterpret_cast<const char*>(sqlite3_column_text(statement, 7));
        response["items"][index]["created_at"] = ca ? ca : "";
        const char* imgPath = reinterpret_cast<const char*>(sqlite3_column_text(statement, 8));
        response["items"][index]["image_path"] = imgPath ? imgPath : "";
        index++;
    }

    sqlite3_finalize(statement);
    response["count"] = index;
    return crow::response(200, response);
}

crow::response getAdminItems(
    const crow::request& req
)
{
    AuthUser user;

    // Check authentication
    if (!authenticateUser(req, user))
    {
        return crow::response(
            401,
            "Unauthorized"
        );
    }

    // Check admin role
    if (user.role != "admin")
    {
        return crow::response(
            403,
            "Forbidden: Admin access required"
        );
    }


    // Get all items
    const char* query =
        "SELECT id, title, description, type, location, "
        "status, user_id, created_at, image_path "
        "FROM items "
        "ORDER BY id DESC;";


    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );


    if (result != SQLITE_OK)
    {
        return crow::response(
            500,
            "Database query preparation failed"
        );
    }


    crow::json::wvalue response;

    int index = 0;


    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        response["items"][index]["id"] =
            sqlite3_column_int(statement, 0);

        response["items"][index]["title"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 1)
            );

        response["items"][index]["description"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 2)
            );

        response["items"][index]["type"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 3)
            );

        response["items"][index]["location"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 4)
            );

        response["items"][index]["status"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 5)
            );

        response["items"][index]["user_id"] =
            sqlite3_column_int(statement, 6);

        response["items"][index]["created_at"] =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 7)
            );

        const char* imgPath = reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 8)
        );
        response["items"][index]["image_path"] = imgPath ? imgPath : "";

        index++;
    }


    sqlite3_finalize(statement);


    response["count"] = index;


    return crow::response(
        200,
        response
    );
}

crow::response getUploadedFile(const crow::request& req, const std::string& filename)
{
    // Prevent directory traversal attacks
    if (filename.empty() ||
        filename.find("..") != std::string::npos ||
        filename.find('/') != std::string::npos ||
        filename.find('\\') != std::string::npos)
    {
        return crow::response(400, "Invalid filename");
    }

    std::string filePath = "uploads/" + filename;
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return crow::response(404, "Image not found");
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();

    crow::response resp(200, data);

    std::string ext = getFileExtension(filename);
    if (ext == ".png")
    {
        resp.set_header("Content-Type", "image/png");
    }
    else if (ext == ".jpg" || ext == ".jpeg")
    {
        resp.set_header("Content-Type", "image/jpeg");
    }
    else
    {
        resp.set_header("Content-Type", "application/octet-stream");
    }

    resp.set_header("Cache-Control", "public, max-age=86400");
    return resp;
}
