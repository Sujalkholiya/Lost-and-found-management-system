#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    void setActiveScreen(int index);
    void setActiveAdminTab(int tabIndex);
    void updateAuthStatus(bool isLoggedIn, const QString &userName, const QString &userRole);

signals:
    void navigateTo(int screenIndex);
    void navigateToAdminTab(int tabIndex);
    void logoutRequested();
    void profileClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    QPushButton* createNavButton(const QString &text, const QString &iconText, int screenIndex);
    void updateAvatar(const QString &name);

    QVBoxLayout *m_layout;
    QList<QPushButton*> m_navButtons;
    QPushButton *m_overviewBtn;
    QPushButton *m_browseBtn;
    QPushButton *m_reportBtn;
    QPushButton *m_dashboardBtn;
    QPushButton *m_claimsBtn;
    QPushButton *m_adminBtn;
    QPushButton *m_claimMgmtBtn;
    QPushButton *m_itemInvBtn;
    QPushButton *m_accountBtn;
    QPushButton *m_loginBtn;
    QPushButton *m_logoutBtn;
    QPushButton *m_profileCardBtn;
    QLabel *m_userAvatarLabel;
    QLabel *m_userBadgeLabel;
    QLabel *m_userNameLabel;
};

#endif // SIDEBARWIDGET_H
