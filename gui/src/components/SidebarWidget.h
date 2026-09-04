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
    void updateAuthStatus(bool isLoggedIn, const QString &userName, const QString &userRole);

signals:
    void navigateTo(int screenIndex);
    void logoutRequested();

private:
    void setupUI();
    QPushButton* createNavButton(const QString &text, const QString &iconText, int screenIndex);

    QVBoxLayout *m_layout;
    QList<QPushButton*> m_navButtons;
    QPushButton *m_adminBtn;
    QPushButton *m_dashboardBtn;
    QPushButton *m_loginBtn;
    QPushButton *m_logoutBtn;
    QLabel *m_userBadgeLabel;
    QLabel *m_userNameLabel;
};

#endif // SIDEBARWIDGET_H
