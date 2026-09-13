#include "ApiClient.h"
#include "../auth/AuthManager.h"
#include <QUrl>
#include <QDebug>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_baseUrl("http://127.0.0.1:18080")
{
}

ApiClient::~ApiClient()
{
}

QNetworkRequest ApiClient::createRequest(const QString &endpoint, bool requiresAuth)
{
    QUrl url(m_baseUrl + endpoint);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (requiresAuth) {
        QString token = AuthManager::instance().token();
        if (!token.isEmpty()) {
            request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
        }
    }

    return request;
}

// 1. Register User: POST /api/users/register
void ApiClient::registerUser(
    const QString &name,
    const QString &email,
    const QString &password,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    QJsonObject body;
    body["name"] = name;
    body["email"] = email;
    body["password"] = password;

    sendPost("/api/users/register", body, false, onSuccess, onError);
}

// 2. Login User: POST /api/users/login
void ApiClient::loginUser(
    const QString &email,
    const QString &password,
    UserCallback onSuccess,
    ErrorCallback onError)
{
    qDebug() << "[ApiClient] loginUser called for endpoint: POST /api/users/login";
    qDebug() << "[ApiClient] Request URL:" << m_baseUrl + "/api/users/login";

    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    sendPost("/api/users/login", body, false,
        [onSuccess, onError](const QJsonObject &response) {
            bool hasToken = response.contains("token") && !response.value("token").toString().isEmpty();
            bool hasUser = response.contains("user") && response.value("user").isObject();

            qDebug() << "[ApiClient] Login response received. Parsed token status:" << (hasToken ? "SUCCESS (valid token)" : "FAILED (missing token)");
            qDebug() << "[ApiClient] Parsed user object status:" << (hasUser ? "SUCCESS (valid user object)" : "FAILED (missing user)");

            if (!hasToken || !hasUser) {
                qDebug() << "[ApiClient] Login failure: unexpected response format.";
                if (onError) {
                    onError(500, "Unexpected response format from authentication server.");
                }
                return;
            }

            QString token = response.value("token").toString();
            QJsonObject userObj = response.value("user").toObject();
            User user = User::fromJson(userObj);

            qDebug() << "[ApiClient] Parsed User Name:" << user.name << "Email:" << user.email << "Role:" << user.role;

            // Update session state in AuthManager
            AuthManager::instance().setSession(user, token);
            qDebug() << "[ApiClient] AuthManager session successfully updated.";

            if (onSuccess) {
                onSuccess(user, token);
            }
        },
        [onError](int statusCode, const QString &errorMessage) {
            qDebug() << "[ApiClient] loginUser error received. HTTP status code:" << statusCode << "Error message:" << errorMessage;
            if (onError) {
                onError(statusCode, errorMessage);
            }
        }
    );
}

// 3. Get All Items: GET /api/items
void ApiClient::getAllItems(
    ItemsCallback onSuccess,
    ErrorCallback onError)
{
    sendGet("/api/items", false,
        [onSuccess, onError](const QJsonObject &response) {
            int count = response.value("count").toInt(0);
            QJsonArray itemsArray = response.value("items").toArray();
            
            QVector<Item> items;
            for (const QJsonValue &val : itemsArray) {
                items.append(Item::fromJson(val.toObject()));
            }

            if (onSuccess) {
                onSuccess(items, count > 0 ? count : items.size());
            }
        },
        onError
    );
}

// 4. Get Item By ID: GET /api/items/:id
void ApiClient::getItemById(
    int id,
    ItemCallback onSuccess,
    ErrorCallback onError)
{
    QString endpoint = QString("/api/items/%1").arg(id);

    sendGet(endpoint, false,
        [onSuccess, onError](const QJsonObject &response) {
            QJsonObject itemObj;
            if (response.contains("item")) {
                itemObj = response.value("item").toObject();
            } else {
                itemObj = response;
            }

            if (itemObj.isEmpty()) {
                if (onError) onError(404, "Item not found.");
                return;
            }

            Item item = Item::fromJson(itemObj);
            if (onSuccess) {
                onSuccess(item);
            }
        },
        onError
    );
}

// 5. Create Lost/Found Item: POST /api/items (with optional image)
void ApiClient::createItem(
    const QString &title,
    const QString &description,
    const QString &type,
    const QString &location,
    ItemCallback onSuccess,
    ErrorCallback onError,
    const QString &imageFilePath)
{
    auto handleSuccess = [onSuccess, onError](const QJsonObject &response) {
        QJsonObject itemObj = response.value("item").toObject();
        if (itemObj.isEmpty()) {
            itemObj = response;
        }

        Item item = Item::fromJson(itemObj);
        if (onSuccess) {
            onSuccess(item);
        }
    };

    if (!imageFilePath.isEmpty() && QFile::exists(imageFilePath)) {
        emit networkActivityStarted();

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        auto addTextPart = [multiPart](const QString &name, const QString &value) {
            QHttpPart textPart;
            textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QString("form-data; name=\"%1\"").arg(name));
            textPart.setBody(value.toUtf8());
            multiPart->append(textPart);
        };

        addTextPart("title", title);
        addTextPart("description", description);
        addTextPart("type", type.toLower());
        addTextPart("location", location);

        QFile *file = new QFile(imageFilePath, multiPart);
        if (file->open(QIODevice::ReadOnly)) {
            QHttpPart imagePart;
            QFileInfo fileInfo(imageFilePath);
            QString suffix = fileInfo.suffix().toLower();
            QString mimeType = (suffix == "png") ? "image/png" : "image/jpeg";

            imagePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
            imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                QString("form-data; name=\"image\"; filename=\"%1\"").arg(fileInfo.fileName()));
            imagePart.setBodyDevice(file);
            multiPart->append(imagePart);
        }

        QUrl url(m_baseUrl + "/api/items");
        QNetworkRequest request(url);
        if (AuthManager::instance().isLoggedIn()) {
            request.setRawHeader("Authorization", ("Bearer " + AuthManager::instance().token()).toUtf8());
        }

        QNetworkReply *reply = m_manager->post(request, multiPart);
        multiPart->setParent(reply); // Reply will delete multiPart when done

        connect(reply, &QNetworkReply::finished, this, [this, reply, handleSuccess, onError]() {
            emit networkActivityFinished();
            handleReply(reply, handleSuccess, onError);
        });
    } else {
        QJsonObject body;
        body["title"] = title;
        body["description"] = description;
        body["type"] = type.toLower(); // "lost" or "found"
        body["location"] = location;

        sendPost("/api/items", body, true, handleSuccess, onError);
    }
}

// 6. Admin Get All Items: GET /api/admin/items
void ApiClient::getAdminItems(
    ItemsCallback onSuccess,
    ErrorCallback onError)
{
    sendGet("/api/admin/items", true,
        [onSuccess, onError](const QJsonObject &response) {
            int count = response.value("count").toInt(0);
            QJsonArray itemsArray = response.value("items").toArray();

            QVector<Item> items;
            for (const QJsonValue &val : itemsArray) {
                items.append(Item::fromJson(val.toObject()));
            }

            if (onSuccess) {
                onSuccess(items, count > 0 ? count : items.size());
            }
        },
        onError
    );
}

// 7. Delete Item: DELETE /api/items/:id
void ApiClient::deleteItem(
    int itemId,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    sendDelete(QString("/api/items/%1").arg(itemId), true, onSuccess, onError);
}

// 8. Get Current User Profile: GET /api/users/me
void ApiClient::getProfile(
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    sendGet("/api/users/me", true, onSuccess, onError);
}

// 9. Update Current User Profile: PUT /api/users/me
void ApiClient::updateProfile(
    const QString &newName,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    QJsonObject body;
    body["name"] = newName.trimmed();
    sendPut("/api/users/me", body, true, onSuccess, onError);
}

// 10. Get Logged-In User's Items: GET /api/users/me/items
void ApiClient::getUserItems(
    ItemsCallback onSuccess,
    ErrorCallback onError)
{
    sendGet("/api/users/me/items", true,
        [onSuccess, onError](const QJsonObject &response) {
            int count = response.value("count").toInt(0);
            QJsonArray itemsArray = response.value("items").toArray();
            
            QVector<Item> items;
            for (const QJsonValue &val : itemsArray) {
                items.append(Item::fromJson(val.toObject()));
            }

            if (onSuccess) {
                onSuccess(items, count > 0 ? count : items.size());
            }
        },
        onError
    );
}

// 11. Submit Claim for Item: POST /api/items/:id/claims
void ApiClient::submitClaim(
    int itemId,
    const QString &lastSeenLocation,
    const QString &uniqueDetails,
    const QString &itemContents,
    const QString &additionalDetails,
    ClaimCallback onSuccess,
    ErrorCallback onError)
{
    QJsonObject body;
    body["last_seen_location"] = lastSeenLocation;
    body["unique_identifying_details"] = uniqueDetails;
    body["item_contents"] = itemContents;
    if (!additionalDetails.trimmed().isEmpty()) {
        body["additional_details"] = additionalDetails;
    }

    sendPost(QString("/api/items/%1/claims").arg(itemId), body, true,
        [onSuccess](const QJsonObject &response) {
            QJsonObject claimObj;
            if (response.contains("claim") && response.value("claim").isObject()) {
                claimObj = response.value("claim").toObject();
            } else if (response.contains("data") && response.value("data").isObject()) {
                claimObj = response.value("data").toObject();
            } else {
                claimObj = response;
            }

            if (onSuccess) {
                onSuccess(Claim::fromJson(claimObj));
            }
        },
        onError
    );
}

// 12. Get Current User Claims: GET /api/users/me/claims
void ApiClient::getUserClaims(
    ClaimsCallback onSuccess,
    ErrorCallback onError)
{
    sendGet("/api/users/me/claims", true,
        [onSuccess](const QJsonObject &response) {
            int count = response.value("count").toInt(0);
            QJsonArray claimsArray = response.value("claims").toArray();

            QVector<Claim> claims;
            for (const QJsonValue &val : claimsArray) {
                claims.append(Claim::fromJson(val.toObject()));
            }

            if (onSuccess) {
                onSuccess(claims, count > 0 ? count : claims.size());
            }
        },
        onError
    );
}

// 13. Get Current User Claim for specific Item: GET /api/items/:id/my-claim
void ApiClient::getMyClaimForItem(
    int itemId,
    MyClaimCallback onSuccess,
    ErrorCallback onError)
{
    sendGet(QString("/api/items/%1/my-claim").arg(itemId), true,
        [onSuccess](const QJsonObject &response) {
            bool hasClaim = response.value("has_claim").toBool(false);
            Claim claim;
            if (hasClaim) {
                claim = Claim::fromJson(response.value("claim").toObject());
            }
            if (onSuccess) {
                onSuccess(hasClaim, claim);
            }
        },
        onError
    );
}

// 14. Get Claim Details by ID: GET /api/claims/:id
void ApiClient::getClaimById(
    int claimId,
    ClaimCallback onSuccess,
    ErrorCallback onError)
{
    sendGet(QString("/api/claims/%1").arg(claimId), true,
        [onSuccess](const QJsonObject &response) {
            if (onSuccess) {
                onSuccess(Claim::fromJson(response));
            }
        },
        onError
    );
}

// 15. Admin Get All Claims: GET /api/admin/claims
void ApiClient::getAdminClaims(
    const QString &statusFilter,
    ClaimStatsCallback onSuccess,
    ErrorCallback onError)
{
    QString endpoint = "/api/admin/claims";
    if (!statusFilter.trimmed().isEmpty() && statusFilter != "all") {
        endpoint += QString("?status=%1").arg(statusFilter);
    }

    sendGet(endpoint, true,
        [onSuccess](const QJsonObject &response) {
            QJsonArray claimsArray = response.value("claims").toArray();
            QVector<Claim> claims;
            for (const QJsonValue &val : claimsArray) {
                claims.append(Claim::fromJson(val.toObject()));
            }

            ClaimStats stats = ClaimStats::fromJson(response.value("stats").toObject());
            if (onSuccess) {
                onSuccess(claims, stats);
            }
        },
        onError
    );
}

// 16. Admin Approve Claim: PUT /api/admin/claims/:id/approve
void ApiClient::approveClaim(
    int claimId,
    const QString &pickupLocation,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    QJsonObject body;
    if (!pickupLocation.trimmed().isEmpty()) {
        body["pickup_location"] = pickupLocation;
    }

    sendPut(QString("/api/admin/claims/%1/approve").arg(claimId), body, true, onSuccess, onError);
}

// 17. Admin Reject Claim: PUT /api/admin/claims/:id/reject
void ApiClient::rejectClaim(
    int claimId,
    const QString &reason,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    QJsonObject body;
    body["rejection_reason"] = reason;

    sendPut(QString("/api/admin/claims/%1/reject").arg(claimId), body, true, onSuccess, onError);
}

// 18. Admin Handover Item: PUT /api/admin/claims/:id/handover
void ApiClient::handoverClaim(
    int claimId,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    QJsonObject emptyBody;
    sendPut(QString("/api/admin/claims/%1/handover").arg(claimId), emptyBody, true, onSuccess, onError);
}

// Centralized GET dispatcher
void ApiClient::sendGet(
    const QString &endpoint,
    bool requiresAuth,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    emit networkActivityStarted();
    QNetworkRequest request = createRequest(endpoint, requiresAuth);
    QNetworkReply *reply = m_manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        emit networkActivityFinished();
        handleReply(reply, onSuccess, onError);
    });
}

// Centralized POST dispatcher
void ApiClient::sendPost(
    const QString &endpoint,
    const QJsonObject &body,
    bool requiresAuth,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    emit networkActivityStarted();
    QNetworkRequest request = createRequest(endpoint, requiresAuth);
    QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_manager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        emit networkActivityFinished();
        handleReply(reply, onSuccess, onError);
    });
}

// Centralized PUT dispatcher
void ApiClient::sendPut(
    const QString &endpoint,
    const QJsonObject &body,
    bool requiresAuth,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    emit networkActivityStarted();
    QNetworkRequest request = createRequest(endpoint, requiresAuth);
    QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_manager->put(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        emit networkActivityFinished();
        handleReply(reply, onSuccess, onError);
    });
}

// Centralized DELETE dispatcher
void ApiClient::sendDelete(
    const QString &endpoint,
    bool requiresAuth,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    emit networkActivityStarted();
    QNetworkRequest request = createRequest(endpoint, requiresAuth);
    QNetworkReply *reply = m_manager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        emit networkActivityFinished();
        handleReply(reply, onSuccess, onError);
    });
}

// Centralized HTTP response & error classification
void ApiClient::handleReply(
    QNetworkReply *reply,
    JsonCallback onSuccess,
    ErrorCallback onError)
{
    reply->deleteLater();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl requestUrl = reply->request().url();
    QByteArray rawData = reply->readAll();
    QNetworkReply::NetworkError netError = reply->error();

    qDebug() << "[ApiClient] handleReply for URL:" << requestUrl.toString()
             << "| HTTP Status:" << statusCode
             << "| NetworkError:" << netError;

    // Check for network level errors (e.g. connection refused, timeout, host not found)
    if (netError != QNetworkReply::NoError && statusCode == 0) {
        QString errorMsg = extractErrorMessage(0, rawData, netError);
        qDebug() << "[ApiClient] Network level error detected:" << errorMsg;
        if (onError) {
            onError(0, errorMsg);
        }
        return;
    }

    // Success Status Codes (200 OK, 201 Created)
    if (statusCode >= 200 && statusCode < 300) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[ApiClient] JSON Parse Warning for 2xx response:" << parseError.errorString();
            QJsonObject fallback;
            fallback["message"] = QString::fromUtf8(rawData);
            if (onSuccess) onSuccess(fallback);
            return;
        }

        if (doc.isObject()) {
            if (onSuccess) onSuccess(doc.object());
        } else if (doc.isArray()) {
            QJsonObject root;
            root["items"] = doc.array();
            root["count"] = doc.array().size();
            if (onSuccess) onSuccess(root);
        }
        return;
    }

    // HTTP Error Status Codes (400, 401, 403, 404, 500, etc.)
    QString errorMsg = extractErrorMessage(statusCode, rawData, netError);
    qDebug() << "[ApiClient] HTTP error status code:" << statusCode << "Error message:" << errorMsg;
    if (onError) {
        onError(statusCode, errorMsg);
    }
}

QString ApiClient::extractErrorMessage(
    int statusCode,
    const QByteArray &data,
    QNetworkReply::NetworkError netError)
{
    // Try to parse error message from backend JSON
    if (!data.isEmpty()) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("message")) {
                return obj.value("message").toString();
            }
            if (obj.contains("error")) {
                return obj.value("error").toString();
            }
        } else {
            QString plain = QString::fromUtf8(data).trimmed();
            if (!plain.isEmpty() && plain.length() < 200) {
                return plain;
            }
        }
    }

    // Fallback standard error descriptions by status code
    switch (statusCode) {
        case 400:
            return "Bad Request: Please verify the submitted data.";
        case 401:
            return "Unauthorized: Invalid credentials or session expired. Please sign in.";
        case 403:
            return "Forbidden: You do not have permission to perform this action.";
        case 404:
            return "Not Found: The requested resource could not be located.";
        case 500:
            return "Internal Server Error: Backend server encountered an error.";
        default:
            break;
    }

    // Network connection errors
    switch (netError) {
        case QNetworkReply::ConnectionRefusedError:
            return "Connection refused. Please make sure the C++ backend server is running on http://127.0.0.1:18080.";
        case QNetworkReply::HostNotFoundError:
            return "Host not found: Unable to reach backend server.";
        case QNetworkReply::TimeoutError:
            return "Connection timed out. The server is not responding.";
        default:
            return QString("Network communication error (Code %1).").arg(statusCode);
    }
}
