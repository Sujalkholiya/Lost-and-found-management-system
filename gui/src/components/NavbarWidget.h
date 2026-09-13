#ifndef NAVBARWIDGET_H
#define NAVBARWIDGET_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class NavbarWidget : public QFrame
{
    Q_OBJECT

public:
    explicit NavbarWidget(QWidget *parent = nullptr);
    void setActiveScreen(int index);
    void updateAuthStatus(bool isLoggedIn, const QString &userName, const QString &userRole);

signals:
    void navigateTo(int screenIndex);
    void logoutRequested();
    void profileClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void updateAvatar(const QString &name);

    QLabel *m_brandTitle;
    QLabel *m_statusDot;
    QLabel *m_roleBadge;
    QLabel *m_userLabel;
    QLabel *m_avatarLabel;
    QWidget *m_profileTrigger;
    QWidget *m_authContainer;
    QWidget *m_guestContainer;
    QPushButton *m_homeQuickBtn;
    QPushButton *m_browseQuickBtn;
    QPushButton *m_reportQuickBtn;
    QPushButton *m_signInBtn;
    QPushButton *m_registerBtn;
    QPushButton *m_logoutBtn;
};

#endif // NAVBARWIDGET_H