#ifndef CLAIM_H
#define CLAIM_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

struct Claim {
    int id = 0;
    QString claimReference;
    int itemId = 0;
    int userId = 0;
    QString status = "pending"; // "pending", "approved", "handed_over", "rejected"
    QString lastSeenLocation;
    QString uniqueIdentifyingDetails;
    QString itemContents;
    QString additionalDetails;
    QString rejectionReason;
    QString pickupLocation = "Admin Block - Room 102";
    int reviewedBy = 0;
    int handoverAdminId = 0;
    QString createdAt;
    QString reviewedAt;
    QString approvedAt;
    QString handedOverAt;

    // Joined Item Details
    QString itemTitle;
    QString itemDescription;
    QString itemLocation;
    QString itemType;
    QString itemImagePath;
    QString itemStatus;

    // Joined Claimant Details (for Admin view)
    QString claimantName;
    QString claimantEmail;

    bool isPending() const {
        return status.compare("pending", Qt::CaseInsensitive) == 0;
    }

    bool isApproved() const {
        return status.compare("approved", Qt::CaseInsensitive) == 0;
    }

    bool isHandedOver() const {
        return status.compare("handed_over", Qt::CaseInsensitive) == 0;
    }

    bool isRejected() const {
        return status.compare("rejected", Qt::CaseInsensitive) == 0;
    }

    QString statusDisplayName() const {
        if (isPending()) return "Pending Review";
        if (isApproved()) return "Ready for Pickup";
        if (isHandedOver()) return "Handed Over";
        if (isRejected()) return "Rejected";
        return status;
    }

    QString statusColor() const {
        if (isPending()) return "#f59e0b";     // Amber
        if (isApproved()) return "#10b981";    // Emerald / Green
        if (isHandedOver()) return "#6366f1";  // Indigo
        if (isRejected()) return "#ef4444";    // Rose / Red
        return "#64748b";
    }

    QString statusBadgeBg() const {
        if (isPending()) return "rgba(245, 158, 11, 0.15)";
        if (isApproved()) return "rgba(16, 185, 129, 0.15)";
        if (isHandedOver()) return "rgba(99, 102, 241, 0.15)";
        if (isRejected()) return "rgba(239, 68, 68, 0.15)";
        return "rgba(100, 116, 139, 0.15)";
    }

    bool hasImage() const {
        return !itemImagePath.trimmed().isEmpty();
    }

    QString itemImageUrl(const QString& baseUrl = "http://127.0.0.1:18080") const {
        if (!hasImage()) return QString();
        if (itemImagePath.startsWith("http://") || itemImagePath.startsWith("https://")) {
            return itemImagePath;
        }
        QString cleanBase = baseUrl;
        if (cleanBase.endsWith('/')) {
            cleanBase.chop(1);
        }
        return QString("%1/uploads/%2").arg(cleanBase, itemImagePath);
    }

    static Claim fromJson(const QJsonObject& json) {
        Claim c;
        c.id = json.value("id").toInt();

        // Robust multi-key resolution for claim reference
        if (json.contains("claim_reference")) {
            c.claimReference = json.value("claim_reference").toString();
        } else if (json.contains("claimReference")) {
            c.claimReference = json.value("claimReference").toString();
        } else if (json.contains("reference")) {
            c.claimReference = json.value("reference").toString();
        }
        if (c.claimReference.isEmpty() && c.id > 0) {
            c.claimReference = QString("CLM-2026-%1").arg(100 + c.id);
        }

        c.itemId = json.value("item_id").toInt();
        if (c.itemId == 0) c.itemId = json.value("itemId").toInt();

        c.userId = json.value("user_id").toInt();
        if (c.userId == 0) c.userId = json.value("userId").toInt();

        c.status = json.value("status").toString("pending");
        c.lastSeenLocation = json.value("last_seen_location").toString();
        c.uniqueIdentifyingDetails = json.value("unique_identifying_details").toString();
        c.itemContents = json.value("item_contents").toString();
        c.additionalDetails = json.value("additional_details").toString();
        c.rejectionReason = json.value("rejection_reason").toString();
        c.pickupLocation = json.value("pickup_location").toString("Admin Block - Room 102");
        c.reviewedBy = json.value("reviewed_by").toInt();
        c.handoverAdminId = json.value("handover_admin_id").toInt();
        c.createdAt = json.value("created_at").toString();
        c.reviewedAt = json.value("reviewed_at").toString();
        c.approvedAt = json.value("approved_at").toString();
        c.handedOverAt = json.value("handed_over_at").toString();

        // Joined item fields
        c.itemTitle = json.value("item_title").toString();
        c.itemDescription = json.value("item_description").toString();
        c.itemLocation = json.value("item_location").toString();
        c.itemType = json.value("item_type").toString();
        c.itemImagePath = json.value("item_image_path").toString();
        c.itemStatus = json.value("item_status").toString();

        // Claimant fields
        c.claimantName = json.value("claimant_name").toString();
        c.claimantEmail = json.value("claimant_email").toString();

        return c;
    }

    QJsonObject toJson() const {
        QJsonObject json;
        if (id > 0) json["id"] = id;
        if (!claimReference.isEmpty()) json["claim_reference"] = claimReference;
        json["item_id"] = itemId;
        json["last_seen_location"] = lastSeenLocation;
        json["unique_identifying_details"] = uniqueIdentifyingDetails;
        json["item_contents"] = itemContents;
        if (!additionalDetails.isEmpty()) json["additional_details"] = additionalDetails;
        return json;
    }
};

struct ClaimStats {
    int total = 0;
    int pending = 0;
    int approved = 0;
    int handedOver = 0;
    int rejected = 0;

    static ClaimStats fromJson(const QJsonObject& json) {
        ClaimStats s;
        s.total = json.value("total").toInt();
        s.pending = json.value("pending").toInt();
        s.approved = json.value("approved").toInt();
        s.handedOver = json.value("handed_over").toInt();
        s.rejected = json.value("rejected").toInt();
        return s;
    }
};

#endif // CLAIM_H

