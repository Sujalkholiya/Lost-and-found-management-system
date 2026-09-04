#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

struct Item {
    int id = 0;
    QString title;
    QString description;
    QString type;      // "lost" or "found"
    QString location;
    QString status = "active";
    int userId = 0;
    QString createdAt;
    QString imagePath;

    bool isLost() const {
        return type.toLower() == "lost";
    }

    bool isFound() const {
        return type.toLower() == "found";
    }

    bool hasImage() const {
        return !imagePath.trimmed().isEmpty();
    }

    QString imageUrl(const QString& baseUrl = "http://127.0.0.1:18080") const {
        if (!hasImage()) return QString();
        if (imagePath.startsWith("http://") || imagePath.startsWith("https://")) {
            return imagePath;
        }
        QString cleanBase = baseUrl;
        if (cleanBase.endsWith('/')) {
            cleanBase.chop(1);
        }
        return QString("%1/uploads/%2").arg(cleanBase, imagePath);
    }

    static Item fromJson(const QJsonObject& json) {
        Item item;
        item.id = json.value("id").toInt();
        item.title = json.value("title").toString();
        item.description = json.value("description").toString();
        item.type = json.value("type").toString();
        item.location = json.value("location").toString();
        item.status = json.value("status").toString("active");
        item.userId = json.value("user_id").toInt();
        item.createdAt = json.value("created_at").toString();
        item.imagePath = json.value("image_path").toString();
        return item;
    }

    QJsonObject toJson() const {
        QJsonObject json;
        json["id"] = id;
        json["title"] = title;
        json["description"] = description;
        json["type"] = type;
        json["location"] = location;
        if (!status.isEmpty()) json["status"] = status;
        if (userId > 0) json["user_id"] = userId;
        if (!createdAt.isEmpty()) json["created_at"] = createdAt;
        if (!imagePath.isEmpty()) json["image_path"] = imagePath;
        return json;
    }
};

#endif // ITEM_H

