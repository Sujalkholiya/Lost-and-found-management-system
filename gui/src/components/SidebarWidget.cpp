#include "SidebarWidget.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QVariant>
#include <QStyle>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_adminBtn(nullptr)
    , m_dashboardBtn(nullptr)
    , m_loginBtn(nullptr)
    , m_logoutBtn(nullptr)
    , m_userBadgeLabel(nullptr)
    , m_userNameLabel(nullptr)
{
    setupUI();
}

void SidebarWidget::setupUI()
{
    setFixedWidth(240);
    setStyleSheet(
        "SidebarWidget {"
        "   background-color: #0F172A;"
        "   border-right: 1px solid #1E293B;"
        "}"
    );

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(16, 24, 16, 20);
    m_layout->setSpacing(8);

    // Brand Logo & Header
    QWidget *brandWidget = new QWidget(this);
    QHBoxLayout *brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(4, 0, 4, 16);
    brandLayout->setSpacing(12);

    QLabel *logoIcon = new QLabel("🎓", brandWidget);
    logoIcon->setStyleSheet("font-size: 28px; background: transparent;");
    
    QVBoxLayout *brandTextLayout = new QVBoxLayout();
    brandTextLayout->setSpacing(2);
    
    QLabel *brandTitle = new QLabel("Campus Portal", brandWidget);
    brandTitle->setStyleSheet("font-size: 16px; font-weight: 800; color: #FFFFFF; background: transparent;");
    
    QLabel *brandSub = new QLabel("Lost & Found Network", brandWidget);
    brandSub->setStyleSheet("font-size: 11px; color: #818CF8; font-weight: 600; background: transparent;");

    brandTextLayout->addWidget(brandTitle);
    brandTextLayout->addWidget(brandSub);
    brandLayout->addWidget(logoIcon);
    brandLayout->addLayout(brandTextLayout);
    brandLayout->addStretch();
    m_layout->addWidget(brandWidget);

    // Divider
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #1E293B; margin-bottom: 8px;");
    m_layout->addWidget(line);

    // Navigation Buttons
    // Index 0: Home
    m_layout->addWidget(createNavButton("Home Overview", "🏠", 0));
    // Index 1: Browse Items
    m_layout->addWidget(createNavButton("Browse Items", "🔍", 1));
    // Index 2: Report Item
    m_layout->addWidget(createNavButton("Report Item", "➕", 2));
    // Index 5: User Dashboard
    m_dashboardBtn = createNavButton("My Dashboard", "👤", 5);
    m_layout->addWidget(m_dashboardBtn);

    // Index 6: Admin Panel
    m_adminBtn = createNavButton("Admin Console", "🛡️", 6);
    m_adminBtn->setVisible(false); // Shown only for admin role
    m_layout->addWidget(m_adminBtn);

    m_layout->addStretch();

    // User Profile Pill Section at bottom
    QFrame *userFrame = new QFrame(this);
    userFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #1E293B;"
        "   border: 1px solid #334155;"
        "   border-radius: 10px;"
        "   padding: 12px;"
        "}"
    );
    QVBoxLayout *userLayout = new QVBoxLayout(userFrame);
    userLayout->setContentsMargins(8, 8, 8, 8);
    userLayout->setSpacing(4);

    m_userNameLabel = new QLabel("Guest Visitor", userFrame);
    m_userNameLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #F8FAFC; background: transparent;");
    
    m_userBadgeLabel = new QLabel("Not Signed In", userFrame);
    m_userBadgeLabel->setStyleSheet("font-size: 11px; font-weight: 500; color: #94A3B8; background: transparent;");

    userLayout->addWidget(m_userNameLabel);
    userLayout->addWidget(m_userBadgeLabel);
    m_layout->addWidget(userFrame);

    // Auth Action Buttons
    m_loginBtn = new QPushButton("🔑 Sign In", this);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(m_loginBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(3); }); // 3 = Login
    m_layout->addWidget(m_loginBtn);

    m_logoutBtn = new QPushButton("🚪 Log Out", this);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #F87171;"
        "   border: 1px solid #7F1D1D;"
        "   border-radius: 8px;"
        "   padding: 9px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #7F1D1D; color: #FFFFFF; }"
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
        "   color: #94A3B8;"
        "   border-radius: 8px;"
        "   padding: 11px 14px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   text-align: left;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1E293B;"
        "   color: #FFFFFF;"
        "}"
        "QPushButton[active='true'] {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "}"
    );

    connect(btn, &QPushButton::clicked, this, [this, screenIndex]() {
        emit navigateTo(screenIndex);
    });

    m_navButtons.append(btn);
    return btn;
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
        if (m_userNameLabel) m_userNameLabel->setText(userName.isEmpty() ? "Logged In User" : userName);
        if (m_userBadgeLabel) {
            if (userRole.toLower() == "admin") {
                m_userBadgeLabel->setText("👑 Administrator");
            } else {
                m_userBadgeLabel->setText("🎒 Student Account");
            }
        }
        if (m_adminBtn) m_adminBtn->setVisible(userRole.toLower() == "admin");
        if (m_loginBtn) m_loginBtn->setVisible(false);
        if (m_logoutBtn) m_logoutBtn->setVisible(true);
        if (m_dashboardBtn) m_dashboardBtn->setVisible(true);
    } else {
        if (m_userNameLabel) m_userNameLabel->setText("Guest Visitor");
        if (m_userBadgeLabel) m_userBadgeLabel->setText("Not Signed In");
        if (m_adminBtn) m_adminBtn->setVisible(false);
        if (m_loginBtn) m_loginBtn->setVisible(true);
        if (m_logoutBtn) m_logoutBtn->setVisible(false);
        if (m_dashboardBtn) m_dashboardBtn->setVisible(false);
    }
}