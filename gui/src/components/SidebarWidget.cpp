#include "SidebarWidget.h"
#include "AvatarHelper.h"
#include "ProfileImageManager.h"
#include "../auth/AuthManager.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QVariant>
#include <QStyle>
#include <QPainter>
#include <QStyleOption>
#include <QPalette>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_adminBtn(nullptr)
    , m_dashboardBtn(nullptr)
    , m_claimsBtn(nullptr)
    , m_loginBtn(nullptr)
    , m_logoutBtn(nullptr)
    , m_profileCardBtn(nullptr)
    , m_userAvatarLabel(nullptr)
    , m_userBadgeLabel(nullptr)
    , m_userNameLabel(nullptr)
{
    setupUI();

    connect(&ProfileImageManager::instance(), &ProfileImageManager::profileImageChanged, this, [this](int userId) {
        if (AuthManager::instance().isLoggedIn() && AuthManager::instance().currentUser().id == userId) {
            updateAvatar(AuthManager::instance().currentUser().name);
        }
    });
}

void SidebarWidget::setupUI()
{
    setObjectName("SidebarWidget");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#FFFFFF"));
    setPalette(pal);

    setFixedWidth(250);
    setStyleSheet(
        "#SidebarWidget, SidebarWidget {"
        "   background-color: #FFFFFF;"
        "   border-right: 1px solid #E3EAF3;"
        "}"
    );

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(18, 26, 18, 22);
    m_layout->setSpacing(8);

    // Brand Logo & Header
    QWidget *brandWidget = new QWidget(this);
    QHBoxLayout *brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(4, 0, 4, 18);
    brandLayout->setSpacing(12);

    QLabel *logoIcon = new QLabel("🎓", brandWidget);
    logoIcon->setStyleSheet("font-size: 28px; background: transparent;");
    
    QVBoxLayout *brandTextLayout = new QVBoxLayout();
    brandTextLayout->setSpacing(2);
    
    QLabel *brandTitle = new QLabel("Campus Portal", brandWidget);
    brandTitle->setStyleSheet("font-size: 16px; font-weight: 800; color: #17345F; background: transparent;");
    
    QLabel *brandSub = new QLabel("GEHU Lost & Found", brandWidget);
    brandSub->setStyleSheet("font-size: 12px; color: #6B7FA3; font-weight: 600; background: transparent;");

    brandTextLayout->addWidget(brandTitle);
    brandTextLayout->addWidget(brandSub);
    brandLayout->addWidget(logoIcon);
    brandLayout->addLayout(brandTextLayout);
    brandLayout->addStretch();
    m_layout->addWidget(brandWidget);

    // Category Label: MAIN MENU
    QLabel *navCategoryLabel = new QLabel("MAIN MENU", this);
    navCategoryLabel->setStyleSheet(
        "font-size: 12px; font-weight: 600; color: #7185A5; letter-spacing: 1.5px; background: transparent; padding-left: 8px; margin-top: 8px; margin-bottom: 6px;"
    );
    m_layout->addWidget(navCategoryLabel);

    // Navigation Buttons
    // Index 0: Home Overview
    m_layout->addWidget(createNavButton("Home Overview", "🏠", 0));
    // Index 1: Browse Items
    m_layout->addWidget(createNavButton("Browse Items", "🔍", 1));
    // Index 2: Report Item
    m_layout->addWidget(createNavButton("Report Item", "➕", 2));
    // Index 5: Dashboard
    m_dashboardBtn = createNavButton("My Dashboard", "📊", 5);
    m_dashboardBtn->setVisible(false); // Shown when logged in
    m_layout->addWidget(m_dashboardBtn);

    // Index 8: My Claims
    m_claimsBtn = createNavButton("My Claims", "🎒", 8);
    m_claimsBtn->setVisible(false); // Shown when logged in
    m_layout->addWidget(m_claimsBtn);

    // Index 6: Admin Panel
    m_adminBtn = createNavButton("Admin Console", "🛡️", 6);
    m_adminBtn->setVisible(false); // Shown only for admin role
    m_layout->addWidget(m_adminBtn);

    m_layout->addStretch();

    // User Profile Card at bottom (Clickable trigger for Profile Dialog)
    m_profileCardBtn = new QPushButton(this);
    m_profileCardBtn->setCursor(Qt::PointingHandCursor);
    m_profileCardBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 12px;"
        "   padding: 10px;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #F8FBFF;"
        "   border-color: #CBD5E1;"
        "}"
    );
    m_profileCardBtn->setToolTip("Click to view and edit profile");

    QHBoxLayout *userOuterLayout = new QHBoxLayout(m_profileCardBtn);
    userOuterLayout->setContentsMargins(4, 2, 4, 2);
    userOuterLayout->setSpacing(10);

    m_userAvatarLabel = new QLabel(m_profileCardBtn);
    m_userAvatarLabel->setFixedSize(36, 36);
    m_userAvatarLabel->setStyleSheet("background: transparent; border: none;");

    QVBoxLayout *userLayout = new QVBoxLayout();
    userLayout->setContentsMargins(0, 0, 0, 0);
    userLayout->setSpacing(2);

    m_userNameLabel = new QLabel("Guest Visitor", m_profileCardBtn);
    m_userNameLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #17345F; background: transparent; border: none;");
    
    m_userBadgeLabel = new QLabel("Not Signed In", m_profileCardBtn);
    m_userBadgeLabel->setStyleSheet("font-size: 11px; font-weight: 500; color: #6B7FA3; background: transparent; border: none;");

    userLayout->addWidget(m_userNameLabel);
    userLayout->addWidget(m_userBadgeLabel);

    userOuterLayout->addWidget(m_userAvatarLabel);
    userOuterLayout->addLayout(userLayout);
    userOuterLayout->addStretch();

    connect(m_profileCardBtn, &QPushButton::clicked, this, &SidebarWidget::profileClicked);
    m_layout->addWidget(m_profileCardBtn);

    // Guest Sign In Button
    m_loginBtn = new QPushButton("Sign In", this);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   border-radius: 10px;"
        "   padding: 10px;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );
    connect(m_loginBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(3); }); // 3 = Login
    m_layout->addWidget(m_loginBtn);

    // Logout Button
    m_logoutBtn = new QPushButton("Log Out", this);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFF7F7;"
        "   color: #E54848;"
        "   border: 1px solid #F3C7C7;"
        "   border-radius: 10px;"
        "   padding: 10px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background-color: #FFF0F0;"
        "   color: #D93636;"
        "   border-color: #E54848;"
        "}"
    );
    m_logoutBtn->setVisible(false);
    connect(m_logoutBtn, &QPushButton::clicked, this, &SidebarWidget::logoutRequested);
    m_layout->addWidget(m_logoutBtn);
}

QPushButton* SidebarWidget::createNavButton(const QString &text, const QString &iconText, int screenIndex)
{
    QPushButton *btn = new QPushButton(iconText + "   " + text, this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("screenIndex", screenIndex);
    btn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #526B91;"
        "   border-radius: 11px;"
        "   padding: 12px 16px;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   text-align: left;"
        "   border: none;"
        "   min-height: 22px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EFF5FF;"
        "   color: #2563EB;"
        "   font-weight: 600;"
        "}"
        "QPushButton[active='true'] {"
        "   background-color: #3B82F6;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "}"
        "QPushButton[active='true']:hover {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "}"
    );

    connect(btn, &QPushButton::clicked, this, [this, screenIndex]() {
        emit navigateTo(screenIndex);
    });

    m_navButtons.append(btn);
    return btn;
}

void SidebarWidget::updateAvatar(const QString &name)
{
    if (!m_userAvatarLabel) return;
    int userId = AuthManager::instance().isLoggedIn() ? AuthManager::instance().currentUser().id : 0;
    QString displayName = name.trimmed().isEmpty() ? "User" : name.trimmed();
    QPixmap avatar = ProfileImageManager::instance().getCircularAvatar(userId, displayName, 36, "#3B82F6");
    m_userAvatarLabel->setPixmap(avatar);
}

void SidebarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SidebarWidget::setActiveScreen(int index)
{
    for (QPushButton *btn : m_navButtons) {
        if (!btn) continue;
        bool match = (btn->property("screenIndex").toInt() == index);
        btn->setProperty("active", match);
        if (btn->style()) {
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
        }
    }
}

void SidebarWidget::updateAuthStatus(bool isLoggedIn, const QString &userName, const QString &userRole)
{
    if (isLoggedIn) {
        QString displayName = userName.trimmed().isEmpty() ? "Student" : userName.trimmed();
        if (m_userNameLabel) m_userNameLabel->setText(displayName);
        updateAvatar(displayName);

        if (m_userBadgeLabel) {
            if (userRole.toLower() == "admin") {
                m_userBadgeLabel->setText("👑 Administrator");
            } else {
                m_userBadgeLabel->setText("🎓 Student Account");
            }
        }
        if (m_profileCardBtn) m_profileCardBtn->setVisible(true);
        if (m_adminBtn) m_adminBtn->setVisible(userRole.toLower() == "admin");
        if (m_dashboardBtn) m_dashboardBtn->setVisible(true);
        if (m_claimsBtn) m_claimsBtn->setVisible(true);
        if (m_loginBtn) m_loginBtn->setVisible(false);
        if (m_logoutBtn) m_logoutBtn->setVisible(true);
    } else {
        if (m_userNameLabel) m_userNameLabel->setText("Guest Visitor");
        if (m_userBadgeLabel) m_userBadgeLabel->setText("Not Signed In");
        updateAvatar("Guest");
        if (m_profileCardBtn) m_profileCardBtn->setVisible(false);
        if (m_adminBtn) m_adminBtn->setVisible(false);
        if (m_dashboardBtn) m_dashboardBtn->setVisible(false);
        if (m_claimsBtn) m_claimsBtn->setVisible(false);
        if (m_loginBtn) m_loginBtn->setVisible(true);
        if (m_logoutBtn) m_logoutBtn->setVisible(false);
    }
}