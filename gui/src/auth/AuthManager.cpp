#include "AuthManager.h"
#include <QSettings>
#include <QDebug>

AuthManager& AuthManager::instance()
{
    static AuthManager inst;
    return inst;
}

AuthManager::AuthManager(QObject *parent)
    : QObject(parent)
{
    loadSession();
}

bool AuthManager::isLoggedIn() const
{
    return !m_token.isEmpty() && m_currentUser.isValid();
}

bool AuthManager::isAuthenticated() const
{
    return isLoggedIn();
}

bool AuthManager::isAdmin() const
{
    return isLoggedIn() && m_currentUser.isAdmin();
}

User AuthManager::currentUser() const
{
    return m_currentUser;
}

int AuthManager::userId() const
{
    return m_currentUser.id;
}

QString AuthManager::userName() const
{
    return m_currentUser.name;
}

QString AuthManager::userEmail() const
{
    return m_currentUser.email;
}

QString AuthManager::userRole() const
{
    return m_currentUser.role;
}

QString AuthManager::token() const
{
    return m_token;
}

void AuthManager::setSession(const User &user, const QString &token)
{
    m_currentUser = user;
    m_token = token;

    saveSession();

    qDebug() << "[AuthManager] Session saved. User ID:" << user.id
             << "Name:" << user.name
             << "Role:" << user.role
             << "| Token status: ACTIVE (length" << token.length() << ")";

    emit userLoggedIn(m_currentUser);
    emit authStateChanged(true);
}

void AuthManager::logout()
{
    clearSession();
}

void AuthManager::clearSession()
{
    m_currentUser = User();
    m_token.clear();

    QSettings settings("CollegeLostAndFound", "LostAndFoundGUI");
    settings.remove("auth");

    emit userLoggedOut();
    emit authStateChanged(false);
}

void AuthManager::loadSession()
{
    QSettings settings("CollegeLostAndFound", "LostAndFoundGUI");
    m_token = settings.value("auth/token", "").toString();
    if (!m_token.isEmpty()) {
        m_currentUser.id = settings.value("auth/user_id", 0).toInt();
        m_currentUser.name = settings.value("auth/user_name", "").toString();
        m_currentUser.email = settings.value("auth/user_email", "").toString();
        m_currentUser.role = settings.value("auth/user_role", "student").toString();
    }
}

void AuthManager::saveSession()
{
    QSettings settings("CollegeLostAndFound", "LostAndFoundGUI");
    settings.setValue("auth/token", m_token);
    settings.setValue("auth/user_id", m_currentUser.id);
    settings.setValue("auth/user_name", m_currentUser.name);
    settings.setValue("auth/user_email", m_currentUser.email);
    settings.setValue("auth/user_role", m_currentUser.role);
}
