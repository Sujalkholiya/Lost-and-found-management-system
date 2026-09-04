#ifndef USER_H
#define USER_H

#include <QString>
#include <QJsonObject>

struct User {
    int id = 0;
    QString name;
    QString email;
    QString role; // "student" or "admin"

    bool isValid() const {
        return id > 0 && !email.isEmpty();
    }

    bool isAdmin() const {
        return role.toLower() == "admin";
    }

    static User fromJson(const QJsonObject& json) {
        User u;
        u.id = json.value("id").toInt();
        u.name = json.value("name").toString();
        u.email = json.value("email").toString();
        u.role = json.value("role").toString("student");
        return u;
    }

    QJsonObject toJson() const {
        QJsonObject json;
        json["id"] = id;
        json["name"] = name;
        json["email"] = email;
        json["role"] = role;
        return json;
    }
};

#endif // USER_H

