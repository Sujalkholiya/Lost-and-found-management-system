#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include <QString>
#include "../models/User.h"

class AuthManager : public QObject
{
    Q_OBJECT

public:
    static AuthManager& instance();

    // Session State Queries
    bool isLoggedIn() const;
    bool isAuthenticated() const;
    bool isAdmin() const;

    // Logged-in User Information Getters
    User currentUser() const;
    int userId() const;
    QString userName() const;
    QString userEmail() const;
    QString userRole() const;
    QString token() const;

    // Session State Mutators
    void setSession(const User &user, const QString &token);
    void updateProfile(const QString &newName);
    void logout();
    void clearSession();

signals:
    void authStateChanged(bool isLoggedIn);
    void userLoggedIn(const User &user);
    void userLoggedOut();
    void userProfileUpdated(const User &user);

private:
    explicit AuthManager(QObject *parent = nullptr);
    ~AuthManager() override = default;
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;

    void loadSession();
    void saveSession();

    User m_currentUser;
    QString m_token;
};

#endif // AUTHMANAGER_H
