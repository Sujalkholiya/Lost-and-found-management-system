#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "../network/ApiClient.h"
#include "../auth/AuthManager.h"
#include "../components/NavbarWidget.h"
#include "../components/SidebarWidget.h"
#include "../components/ToastWidget.h"
#include "HomeScreen.h"
#include "BrowseScreen.h"
#include "ReportScreen.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "DashboardScreen.h"
#include "AdminScreen.h"
#include "AccessDeniedScreen.h"
#include "ItemDetailsDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavigate(int screenIndex);
    void onAuthStateChanged(bool isLoggedIn);
    void onShowToast(const QString &message, ToastType type);
    void onShowItemDetails(const Item &item);

private:
    void setupUI();
    void setupConnections();

    ApiClient *m_apiClient;
    AuthManager *m_authManager;

    SidebarWidget *m_sidebarWidget;
    NavbarWidget *m_navbarWidget;
    QStackedWidget *m_stackedWidget;
    ToastWidget *m_toastWidget;

    HomeScreen *m_homeScreen;
    BrowseScreen *m_browseScreen;
    ReportScreen *m_reportScreen;
    LoginScreen *m_loginScreen;
    RegisterScreen *m_registerScreen;
    DashboardScreen *m_dashboardScreen;
    AdminScreen *m_adminScreen;
    AccessDeniedScreen *m_accessDeniedScreen;
};

#endif // MAINWINDOW_H
