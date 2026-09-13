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
#include "../models/Claim.h"

class ApiClient : public QObject
{
    Q_OBJECT

public:
    using JsonCallback = std::function<void(const QJsonObject &response)>;
    using ItemsCallback = std::function<void(const QVector<Item> &items, int count)>;
    using ItemCallback = std::function<void(const Item &item)>;
    using UserCallback = std::function<void(const User &user, const QString &token)>;
    using ClaimsCallback = std::function<void(const QVector<Claim> &claims, int count)>;
    using ClaimCallback = std::function<void(const Claim &claim)>;
    using ClaimStatsCallback = std::function<void(const QVector<Claim> &claims, const ClaimStats &stats)>;
    using MyClaimCallback = std::function<void(bool hasClaim, const Claim &claim)>;
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

    // 8. Get Current User Profile: GET /api/users/me
    void getProfile(
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    // 9. Update Current User Profile: PUT /api/users/me
    void updateProfile(
        const QString &newName,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    // 10. Get Logged-In User's Items: GET /api/users/me/items
    void getUserItems(
        ItemsCallback onSuccess,
        ErrorCallback onError
    );

    // 11. Submit Claim for Item: POST /api/items/:id/claims
    void submitClaim(
        int itemId,
        const QString &lastSeenLocation,
        const QString &uniqueDetails,
        const QString &itemContents,
        const QString &additionalDetails,
        ClaimCallback onSuccess,
        ErrorCallback onError
    );

    // 12. Get Current User Claims: GET /api/users/me/claims
    void getUserClaims(
        ClaimsCallback onSuccess,
        ErrorCallback onError
    );

    // 13. Get Current User Claim for specific Item: GET /api/items/:id/my-claim
    void getMyClaimForItem(
        int itemId,
        MyClaimCallback onSuccess,
        ErrorCallback onError
    );

    // 14. Get Claim Details by ID: GET /api/claims/:id
    void getClaimById(
        int claimId,
        ClaimCallback onSuccess,
        ErrorCallback onError
    );

    // 15. Admin Get All Claims: GET /api/admin/claims
    void getAdminClaims(
        const QString &statusFilter,
        ClaimStatsCallback onSuccess,
        ErrorCallback onError
    );

    // 16. Admin Approve Claim: PUT /api/admin/claims/:id/approve
    void approveClaim(
        int claimId,
        const QString &pickupLocation,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    // 17. Admin Reject Claim: PUT /api/admin/claims/:id/reject
    void rejectClaim(
        int claimId,
        const QString &reason,
        JsonCallback onSuccess,
        ErrorCallback onError
    );

    // 18. Admin Handover Item: PUT /api/admin/claims/:id/handover
    void handoverClaim(
        int claimId,
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

    void sendPut(
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