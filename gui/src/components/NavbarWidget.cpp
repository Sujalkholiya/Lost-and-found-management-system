#include "NavbarWidget.h"
#include <QFrame>
#include <QVBoxLayout>

NavbarWidget::NavbarWidget(QWidget *parent)
    : QFrame(parent)
    , m_brandTitle(nullptr)
    , m_statusDot(nullptr)
    , m_roleBadge(nullptr)
    , m_userLabel(nullptr)
    , m_homeQuickBtn(nullptr)
    , m_browseQuickBtn(nullptr)
    , m_reportQuickBtn(nullptr)
    , m_signInBtn(nullptr)
    , m_registerBtn(nullptr)
    , m_logoutBtn(nullptr)
{
    setupUI();
}

void NavbarWidget::setupUI()
{
    setFixedHeight(64);
    setStyleSheet(
        "NavbarWidget {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #E2E8F0;"
        "}"
    );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(24, 0, 24, 0);
    layout->setSpacing(16);

    // Left: Campus Lost & Found Branding
    QWidget *brandWidget = new QWidget(this);
    QHBoxLayout *brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(0, 0, 0, 0);
    brandLayout->setSpacing(10);

    QLabel *brandIcon = new QLabel("🎓", brandWidget);
    brandIcon->setStyleSheet("font-size: 24px; background: transparent;");

    QVBoxLayout *brandTextLayout = new QVBoxLayout();
    brandTextLayout->setSpacing(0);
    brandTextLayout->setContentsMargins(0, 0, 0, 0);

    m_brandTitle = new QLabel("Campus Lost & Found", brandWidget);
    m_brandTitle->setStyleSheet("font-size: 15px; font-weight: 800; color: #0F172A; background: transparent;");

    QLabel *brandSub = new QLabel("University Portal", brandWidget);
    brandSub->setStyleSheet("font-size: 11px; font-weight: 600; color: #6366F1; background: transparent;");

    brandTextLayout->addWidget(m_brandTitle);
    brandTextLayout->addWidget(brandSub);

    m_statusDot = new QLabel("🟢", brandWidget);
    m_statusDot->setToolTip("Backend Connected (127.0.0.1:18080)");
    m_statusDot->setStyleSheet("font-size: 9px; background: transparent; margin-left: 4px;");

    brandLayout->addWidget(brandIcon);
    brandLayout->addLayout(brandTextLayout);
    brandLayout->addWidget(m_statusDot);
    layout->addWidget(brandWidget);

    layout->addStretch();

    // Center: Quick Navigation
    m_homeQuickBtn = new QPushButton("Home", this);
    m_browseQuickBtn = new QPushButton("Browse", this);
    m_reportQuickBtn = new QPushButton("➕ Report Item", this);

    QString navBtnStyle =
        "QPushButton {"
        "   background: transparent;"
        "   color: #64748B;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 7px 14px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #F1F5F9; color: #0F172A; }";

    m_homeQuickBtn->setStyleSheet(navBtnStyle);
    m_browseQuickBtn->setStyleSheet(navBtnStyle);
    m_reportQuickBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #EEF2FF;"
        "   color: #4F46E5;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 7px 14px;"
        "   border-radius: 8px;"
        "   border: 1px solid #C7D2FE;"
        "}"
        "QPushButton:hover { background-color: #4F46E5; color: #FFFFFF; }"
    );

    m_homeQuickBtn->setCursor(Qt::PointingHandCursor);
    m_browseQuickBtn->setCursor(Qt::PointingHandCursor);
    m_reportQuickBtn->setCursor(Qt::PointingHandCursor);

    connect(m_homeQuickBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(0); });
    connect(m_browseQuickBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); });
    connect(m_reportQuickBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });

    layout->addWidget(m_homeQuickBtn);
    layout->addWidget(m_browseQuickBtn);
    layout->addWidget(m_reportQuickBtn);

    layout->addStretch();

    // Right: Authentication / User Area
    m_userLabel = new QLabel("Guest Visitor", this);
    m_userLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #0F172A;");

    m_roleBadge = new QLabel("GUEST", this);
    m_roleBadge->setStyleSheet(
        "font-size: 10px; font-weight: 800; color: #64748B; background-color: #F1F5F9; padding: 3px 8px; border-radius: 6px; border: 1px solid #E2E8F0;"
    );

    m_signInBtn = new QPushButton("Sign In", this);
    m_signInBtn->setCursor(Qt::PointingHandCursor);
    m_signInBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 6px 14px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(m_signInBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(3); }); // 3 = Login

    m_registerBtn = new QPushButton("Register", this);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 6px 12px;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(m_registerBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(4); }); // 4 = Register

    m_logoutBtn = new QPushButton("Log Out", this);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #DC2626;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 6px 12px;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #FEE2E2; }"
    );
    m_logoutBtn->setVisible(false);
    connect(m_logoutBtn, &QPushButton::clicked, this, &NavbarWidget::logoutRequested);

    layout->addWidget(m_userLabel);
    layout->addWidget(m_roleBadge);
    layout->addWidget(m_signInBtn);
    layout->addWidget(m_registerBtn);
    layout->addWidget(m_logoutBtn);
}

void NavbarWidget::setActiveScreen(int index)
{
    Q_UNUSED(index);
}

void NavbarWidget::updateAuthStatus(bool isLoggedIn, const QString &userName, const QString &userRole)
{
    if (!m_userLabel || !m_roleBadge || !m_logoutBtn || !m_signInBtn || !m_registerBtn) return;

    if (isLoggedIn) {
        m_userLabel->setText(userName.isEmpty() ? "Logged In User" : userName);
        if (userRole.toLower() == "admin") {
            m_roleBadge->setText("👑 ADMIN");
            m_roleBadge->setStyleSheet(
                "font-size: 10px; font-weight: 800; color: #7C3AED; background-color: #F5F3FF; padding: 3px 8px; border-radius: 6px; border: 1px solid #DDD6FE;"
            );
        } else {
            m_roleBadge->setText("🎒 STUDENT");
            m_roleBadge->setStyleSheet(
                "font-size: 10px; font-weight: 800; color: #2563EB; background-color: #EFF6FF; padding: 3px 8px; border-radius: 6px; border: 1px solid #BFDBFE;"
            );
        }
        m_signInBtn->setVisible(false);
        m_registerBtn->setVisible(false);
        m_logoutBtn->setVisible(true);
    } else {
        m_userLabel->setText("Guest Visitor");
        m_roleBadge->setText("GUEST");
        m_roleBadge->setStyleSheet(
            "font-size: 10px; font-weight: 800; color: #64748B; background-color: #F1F5F9; padding: 3px 8px; border-radius: 6px; border: 1px solid #E2E8F0;"
        );
        m_signInBtn->setVisible(true);
        m_registerBtn->setVisible(true);
        m_logoutBtn->setVisible(false);
    }
}