#include "NavbarWidget.h"
#include "AvatarHelper.h"
#include "ProfileImageManager.h"
#include "../auth/AuthManager.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QStyle>
#include <QMouseEvent>

NavbarWidget::NavbarWidget(QWidget *parent)
    : QFrame(parent)
    , m_brandTitle(nullptr)
    , m_statusDot(nullptr)
    , m_roleBadge(nullptr)
    , m_userLabel(nullptr)
    , m_avatarLabel(nullptr)
    , m_profileTrigger(nullptr)
    , m_authContainer(nullptr)
    , m_guestContainer(nullptr)
    , m_homeQuickBtn(nullptr)
    , m_browseQuickBtn(nullptr)
    , m_reportQuickBtn(nullptr)
    , m_signInBtn(nullptr)
    , m_registerBtn(nullptr)
    , m_logoutBtn(nullptr)
{
    setupUI();

    connect(&ProfileImageManager::instance(), &ProfileImageManager::profileImageChanged, this, [this](int userId) {
        Q_UNUSED(userId);
        User u = AuthManager::instance().currentUser();
        updateAvatar(u.name);
    });
}

void NavbarWidget::setupUI()
{
    setFixedHeight(78);
    setStyleSheet(
        "NavbarWidget {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #E2EAF4;"
        "   font-family: 'Segoe UI', -apple-system, sans-serif;"
        "}"
    );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(32, 0, 32, 0);
    layout->setSpacing(0);

    // ==========================================
    // 1. LEFT: Clean Campus Branding (No "Live" dot)
    // ==========================================
    QWidget *brandWidget = new QWidget(this);
    QHBoxLayout *brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(0, 0, 0, 0);
    brandLayout->setSpacing(8);
    brandLayout->setAlignment(Qt::AlignVCenter);

    m_brandTitle = new QLabel("Campus Lost & Found", brandWidget);
    m_brandTitle->setStyleSheet("font-size: 15px; font-weight: 800; color: #17345F; background: transparent;");

    QLabel *brandDot = new QLabel("•", brandWidget);
    brandDot->setStyleSheet("font-size: 14px; font-weight: 700; color: #8A9AB5; background: transparent;");

    QLabel *brandSub = new QLabel("University Portal", brandWidget);
    brandSub->setStyleSheet("font-size: 12px; font-weight: 600; color: #6B7FA3; background: transparent;");

    brandLayout->addWidget(m_brandTitle);
    brandLayout->addWidget(brandDot);
    brandLayout->addWidget(brandSub);
    layout->addWidget(brandWidget);

    // Subtle hidden status dot to preserve object pointer if referenced elsewhere
    m_statusDot = new QLabel(this);
    m_statusDot->setVisible(false);

    layout->addStretch(1);

    // ==========================================
    // 2. CENTER: Navigation Buttons (Soft Blue Pill style)
    // ==========================================
    QWidget *centerNavWidget = new QWidget(this);
    QHBoxLayout *centerNavLayout = new QHBoxLayout(centerNavWidget);
    centerNavLayout->setContentsMargins(0, 0, 0, 0);
    centerNavLayout->setSpacing(8);
    centerNavLayout->setAlignment(Qt::AlignVCenter);

    m_homeQuickBtn = new QPushButton("Home", centerNavWidget);
    m_browseQuickBtn = new QPushButton("Browse", centerNavWidget);
    m_reportQuickBtn = new QPushButton("Report Item", centerNavWidget);

    QString navBtnStyle =
        "QPushButton {"
        "   background: transparent;"
        "   color: #6B7FA3;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 8px 18px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #F4F8FF; color: #2563EB; }"
        "QPushButton[active='true'] { background-color: #EFF5FF; color: #2563EB; font-weight: 700; }";

    m_homeQuickBtn->setStyleSheet(navBtnStyle);
    m_browseQuickBtn->setStyleSheet(navBtnStyle);
    m_reportQuickBtn->setStyleSheet(navBtnStyle);

    m_homeQuickBtn->setCursor(Qt::PointingHandCursor);
    m_browseQuickBtn->setCursor(Qt::PointingHandCursor);
    m_reportQuickBtn->setCursor(Qt::PointingHandCursor);

    connect(m_homeQuickBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(0); });
    connect(m_browseQuickBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); });
    connect(m_reportQuickBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });

    centerNavLayout->addWidget(m_homeQuickBtn);
    centerNavLayout->addWidget(m_browseQuickBtn);
    centerNavLayout->addWidget(m_reportQuickBtn);
    layout->addWidget(centerNavWidget);

    layout->addStretch(1);

    // ==========================================
    // 3. RIGHT: Account Area (Profile + Logout)
    // ==========================================
    QWidget *rightAccountWidget = new QWidget(this);
    QHBoxLayout *rightLayout = new QHBoxLayout(rightAccountWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->setAlignment(Qt::AlignVCenter);

    // --- 3A. Logged-In Container ---
    m_authContainer = new QWidget(rightAccountWidget);
    QHBoxLayout *authLayout = new QHBoxLayout(m_authContainer);
    authLayout->setContentsMargins(0, 0, 0, 0);
    authLayout->setSpacing(24);
    authLayout->setAlignment(Qt::AlignVCenter);

    // Profile Trigger Frame
    QFrame *profileTrigger = new QFrame(m_authContainer);
    profileTrigger->setObjectName("profileTrigger");
    profileTrigger->setCursor(Qt::PointingHandCursor);
    profileTrigger->setStyleSheet(
        "QFrame#profileTrigger {"
        "   background-color: transparent;"
        "   border: 1px solid transparent;"
        "   border-radius: 12px;"
        "   padding: 4px 10px 4px 4px;"
        "}"
        "QFrame#profileTrigger:hover {"
        "   background-color: #F4F8FF;"
        "   border: 1px solid #E2EAF4;"
        "}"
    );
    profileTrigger->setToolTip("Click to view and edit profile");
    profileTrigger->installEventFilter(this);
    m_profileTrigger = profileTrigger;

    QHBoxLayout *profLayout = new QHBoxLayout(profileTrigger);
    profLayout->setContentsMargins(0, 0, 0, 0);
    profLayout->setSpacing(10); // 10-12 px gap between avatar and text
    profLayout->setAlignment(Qt::AlignVCenter);

    // Fixed-size 48x48 circular avatar (within 46-54 px range)
    m_avatarLabel = new QLabel(profileTrigger);
    m_avatarLabel->setFixedSize(48, 48);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet("background: transparent; border: none;");

    // Text block (Name + Role)
    QWidget *textContainer = new QWidget(profileTrigger);
    textContainer->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(1);
    textLayout->setAlignment(Qt::AlignVCenter);

    m_userLabel = new QLabel("User", textContainer);
    m_userLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #172B4D; background: transparent; border: none;");

    m_roleBadge = new QLabel("Student", textContainer);
    m_roleBadge->setStyleSheet("font-size: 11px; font-weight: 600; color: #6B7FA3; background: transparent; border: none;");

    textLayout->addWidget(m_userLabel);
    textLayout->addWidget(m_roleBadge);

    // Dropdown arrow
    QLabel *arrowLabel = new QLabel("▾", profileTrigger);
    arrowLabel->setStyleSheet("font-size: 11px; color: #8A9AB5; font-weight: bold; background: transparent; border: none; margin-left: 2px;");
    arrowLabel->setAlignment(Qt::AlignVCenter);

    profLayout->addWidget(m_avatarLabel);
    profLayout->addWidget(textContainer);
    profLayout->addWidget(arrowLabel);

    // Logout Button
    m_logoutBtn = new QPushButton("Log Out", m_authContainer);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #EF4444;"
        "   font-size: 12px;"
        "   font-weight: 600;"
        "   padding: 7px 16px;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #FEF2F2; color: #DC2626; border-color: #FCA5A5; }"
    );
    connect(m_logoutBtn, &QPushButton::clicked, this, &NavbarWidget::logoutRequested);

    authLayout->addWidget(m_profileTrigger);
    authLayout->addWidget(m_logoutBtn);
    m_authContainer->setVisible(false);

    // --- 3B. Guest Container (Sign In / Register) ---
    m_guestContainer = new QWidget(rightAccountWidget);
    QHBoxLayout *guestLayout = new QHBoxLayout(m_guestContainer);
    guestLayout->setContentsMargins(0, 0, 0, 0);
    guestLayout->setSpacing(10);
    guestLayout->setAlignment(Qt::AlignVCenter);

    m_signInBtn = new QPushButton("Sign In", m_guestContainer);
    m_signInBtn->setCursor(Qt::PointingHandCursor);
    m_signInBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: #172B4D;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 8px 16px;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #F4F8FF; color: #2563EB; border-color: #CBD5E1; }"
    );
    connect(m_signInBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(3); }); // 3 = Login

    m_registerBtn = new QPushButton("Register", m_guestContainer);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 8px 18px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );
    connect(m_registerBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(4); }); // 4 = Register

    guestLayout->addWidget(m_signInBtn);
    guestLayout->addWidget(m_registerBtn);

    rightLayout->addWidget(m_authContainer);
    rightLayout->addWidget(m_guestContainer);
    layout->addWidget(rightAccountWidget);
}

bool NavbarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_profileTrigger && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            emit profileClicked();
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void NavbarWidget::updateAvatar(const QString &name)
{
    if (!m_avatarLabel) return;
    User u = AuthManager::instance().currentUser();
    QString displayName = name.trimmed().isEmpty() ? u.name : name.trimmed();
    QPixmap avatar = ProfileImageManager::instance().getCircularAvatar(u.id, displayName, 48, "#2563EB");
    m_avatarLabel->setPixmap(avatar);
}

void NavbarWidget::setActiveScreen(int index)
{
    if (m_homeQuickBtn) {
        m_homeQuickBtn->setProperty("active", index == 0);
        m_homeQuickBtn->style()->unpolish(m_homeQuickBtn);
        m_homeQuickBtn->style()->polish(m_homeQuickBtn);
    }
    if (m_browseQuickBtn) {
        m_browseQuickBtn->setProperty("active", index == 1);
        m_browseQuickBtn->style()->unpolish(m_browseQuickBtn);
        m_browseQuickBtn->style()->polish(m_browseQuickBtn);
    }
    if (m_reportQuickBtn) {
        m_reportQuickBtn->setProperty("active", index == 2);
        m_reportQuickBtn->style()->unpolish(m_reportQuickBtn);
        m_reportQuickBtn->style()->polish(m_reportQuickBtn);
    }
}

void NavbarWidget::updateAuthStatus(bool isLoggedIn, const QString &userName, const QString &userRole)
{
    if (!m_authContainer || !m_guestContainer || !m_userLabel || !m_roleBadge) return;

    if (isLoggedIn) {
        QString displayName = userName.trimmed().isEmpty() ? "Student" : userName.trimmed();
        m_userLabel->setText(displayName);
        updateAvatar(displayName);

        if (userRole.trimmed().toLower() == "admin") {
            m_roleBadge->setText("Administrator");
            m_roleBadge->setStyleSheet("font-size: 11px; font-weight: 600; color: #7C3AED; background: transparent; border: none;");
        } else {
            m_roleBadge->setText("Student");
            m_roleBadge->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748B; background: transparent; border: none;");
        }

        m_authContainer->setVisible(true);
        m_guestContainer->setVisible(false);
    } else {
        m_authContainer->setVisible(false);
        m_guestContainer->setVisible(true);
    }
}