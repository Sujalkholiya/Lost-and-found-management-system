#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include "../models/Item.h"
#include "../models/User.h"

class ApiClient : public QObject
{
    Q_OBJECT

public:
    using JsonCallback = std::function<void(const QJsonObject &response)>;
    using ItemsCallback = std::function<void(const QVector<Item> &items, int count)>;
    using ItemCallback = std::function<void(const Item &item)>;
    using UserCallback = std::function<void(const User &user, const QString &token)>;
    using ErrorCallback = std::function<void(int statusCode, const QString &errorMessage)>;

    explicit ApiClient(QObject *parent = nullptr);
    ~ApiClient();

    // Configuration
    QString baseUrl() const { return m_baseUrl; }
    void setBaseUrl(const QString &url) { m_baseUrl = url; }

    // 1. Register User: POST /api/users/register
    void registerUser(
        const QString &name,
        const QString &email,
        const QString &password,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    // 2. Login User: POST /api/users/login
    void loginUser(
        const QString &email,
        const QString &password,
        UserCallback onSuccess,
        ErrorCallback onError
    );

    // 3. Get All Items: GET /api/items
    void getAllItems(
        ItemsCallback onSuccess,
        ErrorCallback onError
    );

    // 4. Get Item By ID: GET /api/items/:id
    void getItemById(
        int id,
        ItemCallback onSuccess,
        ErrorCallback onError
    );

    // 5. Create Lost/Found Item: POST /api/items (supports optional image file path)
    void createItem(
        const QString &title,
        const QString &description,
        const QString &type,
        const QString &location,
        ItemCallback onSuccess,
        ErrorCallback onError,
        const QString &imageFilePath = QString()
    );

    // 6. Admin Get All Items: GET /api/admin/items
    void getAdminItems(
        ItemsCallback onSuccess,
        ErrorCallback onError
    );

    // 7. Delete Item: DELETE /api/items/:id
    void deleteItem(
        int itemId,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

signals:
    void networkActivityStarted();
    void networkActivityFinished();

private:
    void sendGet(
        const QString &endpoint,
        bool requiresAuth,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    void sendPost(
        const QString &endpoint,
        const QJsonObject &body,
        bool requiresAuth,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    void sendDelete(
        const QString &endpoint,
        bool requiresAuth,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    QNetworkRequest createRequest(const QString &endpoint, bool requiresAuth);
    void handleReply(
        QNetworkReply *reply,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    QString extractErrorMessage(int statusCode, const QByteArray &data, QNetworkReply::NetworkError netError);

    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
};

#endif // APICLIENT_H