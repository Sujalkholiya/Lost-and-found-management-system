#include "MainWindow.h"
#include "AdminScreen.h"
#include "AccessDeniedScreen.h"
#include "ItemDetailsDialog.h"
#include "ProfileDialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

// Custom content container that renders the campus background with a soft translucent overlay
class CampusBackgroundArea : public QWidget
{
public:
    explicit CampusBackgroundArea(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_bgPixmap(":/assets/resources/images/campus-background.png")
    {
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        if (!m_bgPixmap.isNull()) {
            QSize scaledSize = m_bgPixmap.size();
            scaledSize.scale(size(), Qt::KeepAspectRatioByExpanding);

            int x = (width() - scaledSize.width()) / 2;
            int y = (height() - scaledSize.height()) / 2;

            painter.drawPixmap(x, y, scaledSize.width(), scaledSize.height(), m_bgPixmap);
        } else {
            painter.fillRect(rect(), QColor("#F1F5F9"));
        }

        // Soft translucent neutral overlay over campus image for strong text contrast
        painter.fillRect(rect(), QColor(248, 250, 252, 215));
    }

private:
    QPixmap m_bgPixmap;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_apiClient(nullptr)
    , m_authManager(nullptr)
    , m_sidebarWidget(nullptr)
    , m_navbarWidget(nullptr)
    , m_stackedWidget(nullptr)
    , m_toastWidget(nullptr)
    , m_homeScreen(nullptr)
    , m_browseScreen(nullptr)
    , m_reportScreen(nullptr)
    , m_loginScreen(nullptr)
    , m_registerScreen(nullptr)
    , m_dashboardScreen(nullptr)
    , m_adminScreen(nullptr)
    , m_accessDeniedScreen(nullptr)
    , m_myClaimsScreen(nullptr)
{
    qDebug() << "[MainWindow] Initializing Campus Lost & Found GUI...";

    qDebug() << "[MainWindow] Step 1: Initializing ApiClient & AuthManager...";
    m_apiClient = new ApiClient(this);
    m_authManager = &AuthManager::instance();

    qDebug() << "[MainWindow] Step 2: Calling setupUI()...";
    setupUI();

    qDebug() << "[MainWindow] Step 3: Calling setupConnections()...";
    setupConnections();

    qDebug() << "[MainWindow] Step 4: Setting initial screen to Home (Index 0)...";
    onNavigate(0);

    qDebug() << "[MainWindow] Step 5: Updating initial auth status...";
    User user = m_authManager->currentUser();
    if (m_sidebarWidget) m_sidebarWidget->updateAuthStatus(m_authManager->isLoggedIn(), user.name, user.role);
    if (m_navbarWidget) m_navbarWidget->updateAuthStatus(m_authManager->isLoggedIn(), user.name, user.role);

    qDebug() << "[MainWindow] Step 6: GUI initialized successfully.";
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Campus Lost & Found Management System");
    resize(1200, 800);
    setMinimumSize(960, 640);

    qDebug() << "[MainWindow] setupUI: Creating central widget & root layout...";
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    qDebug() << "[MainWindow] setupUI: Creating SidebarWidget...";
    m_sidebarWidget = new SidebarWidget(this);
    rootLayout->addWidget(m_sidebarWidget);

    qDebug() << "[MainWindow] setupUI: Creating content area & layout...";
    CampusBackgroundArea *contentArea = new CampusBackgroundArea(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    qDebug() << "[MainWindow] setupUI: Creating NavbarWidget...";
    m_navbarWidget = new NavbarWidget(this);
    contentLayout->addWidget(m_navbarWidget);

    qDebug() << "[MainWindow] setupUI: Creating QStackedWidget...";
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet("background: transparent;");

    qDebug() << "[MainWindow] setupUI: Instantiating HomeScreen...";
    m_homeScreen = new HomeScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating BrowseScreen...";
    m_browseScreen = new BrowseScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating ReportScreen...";
    m_reportScreen = new ReportScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating LoginScreen...";
    m_loginScreen = new LoginScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating RegisterScreen...";
    m_registerScreen = new RegisterScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating DashboardScreen...";
    m_dashboardScreen = new DashboardScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating AdminScreen...";
    m_adminScreen = new AdminScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Instantiating AccessDeniedScreen...";
    m_accessDeniedScreen = new AccessDeniedScreen(this);

    qDebug() << "[MainWindow] setupUI: Instantiating MyClaimsScreen...";
    m_myClaimsScreen = new MyClaimsScreen(m_apiClient, this);

    qDebug() << "[MainWindow] setupUI: Adding screens to QStackedWidget...";
    m_stackedWidget->addWidget(m_homeScreen);          // Index 0
    m_stackedWidget->addWidget(m_browseScreen);        // Index 1
    m_stackedWidget->addWidget(m_reportScreen);        // Index 2
    m_stackedWidget->addWidget(m_loginScreen);         // Index 3
    m_stackedWidget->addWidget(m_registerScreen);      // Index 4
    m_stackedWidget->addWidget(m_dashboardScreen);     // Index 5
    m_stackedWidget->addWidget(m_adminScreen);         // Index 6
    m_stackedWidget->addWidget(m_accessDeniedScreen);  // Index 7
    m_stackedWidget->addWidget(m_myClaimsScreen);       // Index 8

    contentLayout->addWidget(m_stackedWidget);
    rootLayout->addWidget(contentArea);

    qDebug() << "[MainWindow] setupUI: Creating ToastWidget...";
    m_toastWidget = new ToastWidget(this);
    qDebug() << "[MainWindow] setupUI: Complete.";
}

void MainWindow::setupConnections()
{
    qDebug() << "[MainWindow] setupConnections: Connecting signals & slots...";

    // Sidebar navigation
    connect(m_sidebarWidget, &SidebarWidget::navigateTo, this, &MainWindow::onNavigate);
    connect(m_sidebarWidget, &SidebarWidget::logoutRequested, m_authManager, &AuthManager::logout);
    connect(m_sidebarWidget, &SidebarWidget::profileClicked, this, &MainWindow::onOpenProfileDialog);

    // Top Navbar navigation
    connect(m_navbarWidget, &NavbarWidget::navigateTo, this, &MainWindow::onNavigate);
    connect(m_navbarWidget, &NavbarWidget::logoutRequested, m_authManager, &AuthManager::logout);
    connect(m_navbarWidget, &NavbarWidget::profileClicked, this, &MainWindow::onOpenProfileDialog);

    // Auth Manager state changes
    connect(m_authManager, &AuthManager::authStateChanged, this, &MainWindow::onAuthStateChanged);
    connect(m_authManager, &AuthManager::userProfileUpdated, this, &MainWindow::onUserProfileUpdated);

    // Cross-screen navigation events
    connect(m_homeScreen, &HomeScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_homeScreen, &HomeScreen::viewItemDetails, this, &MainWindow::onShowItemDetails);
    connect(m_homeScreen, &HomeScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_browseScreen, &BrowseScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_browseScreen, &BrowseScreen::viewItemDetails, this, &MainWindow::onShowItemDetails);
    connect(m_browseScreen, &BrowseScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_reportScreen, &ReportScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_reportScreen, &ReportScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_loginScreen, &LoginScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_loginScreen, &LoginScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_registerScreen, &RegisterScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_registerScreen, &RegisterScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_dashboardScreen, &DashboardScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_dashboardScreen, &DashboardScreen::showToast, this, &MainWindow::onShowToast);
    connect(m_dashboardScreen, &DashboardScreen::viewItemDetails, this, &MainWindow::onShowItemDetails);

    connect(m_adminScreen, &AdminScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_myClaimsScreen, &MyClaimsScreen::navigateTo, this, &MainWindow::onNavigate);
    connect(m_myClaimsScreen, &MyClaimsScreen::showToast, this, &MainWindow::onShowToast);

    connect(m_accessDeniedScreen, &AccessDeniedScreen::homeRequested, this, [this]() { onNavigate(0); });
    connect(m_accessDeniedScreen, &AccessDeniedScreen::loginRequested, this, [this]() { onNavigate(3); });

    qDebug() << "[MainWindow] setupConnections: Complete.";
}

void MainWindow::onNavigate(int screenIndex)
{
    qDebug() << "[MainWindow] onNavigate to screen index:" << screenIndex;

    // Permission guard for Admin Screen (Index 6)
    if (screenIndex == 6) {
        if (!m_authManager->isAdmin()) {
            m_stackedWidget->setCurrentIndex(7); // Access Denied screen
            m_sidebarWidget->setActiveScreen(-1);
            m_navbarWidget->setActiveScreen(-1);
            return;
        }
    }

    // Permission guard for Dashboard Screen (Index 5)
    if (screenIndex == 5) {
        if (!m_authManager->isLoggedIn()) {
            onShowToast("Please sign in to view your dashboard", ToastType::Info);
            m_stackedWidget->setCurrentIndex(3); // Login screen
            m_sidebarWidget->setActiveScreen(3);
            m_navbarWidget->setActiveScreen(3);
            return;
        }
    }

    // Permission guard for My Claims Screen (Index 8)
    if (screenIndex == 8) {
        if (!m_authManager->isLoggedIn()) {
            onShowToast("Please sign in to view your claims and pickup passes", ToastType::Info);
            m_stackedWidget->setCurrentIndex(3); // Login screen
            m_sidebarWidget->setActiveScreen(3);
            m_navbarWidget->setActiveScreen(3);
            return;
        }
    }

    m_stackedWidget->setCurrentIndex(screenIndex);
    m_sidebarWidget->setActiveScreen(screenIndex);
    m_navbarWidget->setActiveScreen(screenIndex);

    // Trigger data refresh when entering screens
    if (screenIndex == 0) m_homeScreen->refreshData();
    if (screenIndex == 1) m_browseScreen->refreshData();
    if (screenIndex == 5) m_dashboardScreen->refreshData();
    if (screenIndex == 6) m_adminScreen->refreshData();
    if (screenIndex == 8) m_myClaimsScreen->refreshData();
}

void MainWindow::onAuthStateChanged(bool isLoggedIn)
{
    User user = m_authManager->currentUser();
    m_sidebarWidget->updateAuthStatus(isLoggedIn, user.name, user.role);
    m_navbarWidget->updateAuthStatus(isLoggedIn, user.name, user.role);

    if (isLoggedIn) {
        onShowToast(QString("Welcome back, %1!").arg(user.name.isEmpty() ? "User" : user.name), ToastType::Success);
    } else {
        onShowToast("You have been signed out.", ToastType::Info);
        if (m_stackedWidget->currentIndex() == 5 || m_stackedWidget->currentIndex() == 6 || m_stackedWidget->currentIndex() == 8) {
            onNavigate(0); // Redirect to Home
        }
    }
}

void MainWindow::onShowToast(const QString &message, ToastType type)
{
    if (m_toastWidget) {
        m_toastWidget->showToast(message, type);
    }
}

void MainWindow::onShowItemDetails(const Item &item)
{
    ItemDetailsDialog dialog(item, m_apiClient, this);
    connect(&dialog, &ItemDetailsDialog::itemDeleted, this, [this](int itemId) {
        Q_UNUSED(itemId);
        onShowToast("Item deleted successfully.", ToastType::Success);
        m_browseScreen->refreshData();
        m_homeScreen->refreshData();
        m_dashboardScreen->refreshData();
    });
    connect(&dialog, &ItemDetailsDialog::claimSubmitted, this, [this](int itemId) {
        Q_UNUSED(itemId);
        onShowToast("Claim request logged! Track live status in My Claims.", ToastType::Success);
        m_browseScreen->refreshData();
        m_homeScreen->refreshData();
        m_dashboardScreen->refreshData();
        if (m_myClaimsScreen) m_myClaimsScreen->refreshData();
    });
    dialog.exec();
}

void MainWindow::onOpenProfileDialog()
{
    if (!m_authManager->isLoggedIn()) {
        onShowToast("Please sign in to view your profile.", ToastType::Info);
        onNavigate(3); // Login screen
        return;
    }

    ProfileDialog dialog(m_apiClient, this);
    dialog.exec();
}

void MainWindow::onUserProfileUpdated(const User &user)
{
    m_sidebarWidget->updateAuthStatus(true, user.name, user.role);
    m_navbarWidget->updateAuthStatus(true, user.name, user.role);
    m_dashboardScreen->refreshData();
    onShowToast("Profile name updated successfully!", ToastType::Success);
}