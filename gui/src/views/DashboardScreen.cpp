#include "DashboardScreen.h"
#include "../auth/AuthManager.h"
#include "../components/Theme.h"
#include "../components/AvatarHelper.h"
#include "../components/ProfileImageManager.h"
#include "../network/ImageLoader.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QTime>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>

DashboardScreen::DashboardScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_greetingLabel(nullptr)
    , m_subGreetingLabel(nullptr)
    , m_profileAvatarLabel(nullptr)
    , m_profileNameLabel(nullptr)
    , m_profileRoleBadge(nullptr)
    , m_profileAffiliationLabel(nullptr)
    , m_profileIdLabel(nullptr)
    , m_changeProfileBtn(nullptr)
    , m_statTotalVal(nullptr)
    , m_statLostVal(nullptr)
    , m_statFoundVal(nullptr)
    , m_statResolvedVal(nullptr)
    , m_claimsCard(nullptr)
    , m_claimsCardLayout(nullptr)
    , m_reportsCountBadge(nullptr)
    , m_reportsTableCard(nullptr)
    , m_reportsRowsLayout(nullptr)
{
    setupUI();
}

QString DashboardScreen::getGreeting() const
{
    int hour = QTime::currentTime().hour();
    if (hour < 12) {
        return "Good morning";
    } else if (hour < 17) {
        return "Good afternoon";
    } else {
        return "Good evening";
    }
}

void DashboardScreen::setupUI()
{
    // Outer Root Layout with natural scroll area
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "   width: 8px;"
        "   background: transparent;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #CBD5E1;"
        "   border-radius: 4px;"
        "   min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #94A3B8;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );

    QWidget *contentWidget = new QWidget(scrollArea);
    contentWidget->setObjectName("dashboardContentWidget");
    contentWidget->setStyleSheet("#dashboardContentWidget { background: transparent; }");

    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(36, 28, 36, 40);
    contentLayout->setSpacing(28);

    // =========================================================================
    // 1. DASHBOARD HEADER / WELCOME
    // =========================================================================
    QVBoxLayout *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(6);

    QLabel *pageTitle = new QLabel("My Dashboard", contentWidget);
    pageTitle->setStyleSheet("font-size: 28px; font-weight: 800; color: #0F172A; background: transparent;");

    m_greetingLabel = new QLabel(contentWidget);
    m_greetingLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #334155; background: transparent;");

    m_subGreetingLabel = new QLabel(
        "Track your lost and found activity, manage your reports, and monitor your claims from one place.",
        contentWidget
    );
    m_subGreetingLabel->setStyleSheet("font-size: 14px; font-weight: 400; color: #64748B; background: transparent; line-height: 1.4;");
    m_subGreetingLabel->setWordWrap(true);

    headerLayout->addWidget(pageTitle);
    headerLayout->addWidget(m_greetingLabel);
    headerLayout->addWidget(m_subGreetingLabel);
    contentLayout->addLayout(headerLayout);

    // =========================================================================
    // 2. LARGE PROFILE CARD
    // =========================================================================
    QFrame *profileCard = new QFrame(contentWidget);
    profileCard->setObjectName("largeProfileCard");
    profileCard->setStyleSheet(
        "QFrame#largeProfileCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 18px;"
        "}"
    );

    QHBoxLayout *profLayout = new QHBoxLayout(profileCard);
    profLayout->setContentsMargins(28, 24, 28, 24);
    profLayout->setSpacing(26);

    // Profile Avatar (96x96 circular)
    m_profileAvatarLabel = new QLabel(profileCard);
    m_profileAvatarLabel->setFixedSize(96, 96);
    m_profileAvatarLabel->setAlignment(Qt::AlignCenter);
    m_profileAvatarLabel->setStyleSheet("background: transparent; border: none;");
    profLayout->addWidget(m_profileAvatarLabel);

    // Profile Information
    QVBoxLayout *profInfoLayout = new QVBoxLayout();
    profInfoLayout->setSpacing(6);

    QHBoxLayout *nameRoleRow = new QHBoxLayout();
    nameRoleRow->setSpacing(12);

    m_profileNameLabel = new QLabel("Student Name", profileCard);
    m_profileNameLabel->setStyleSheet("font-size: 22px; font-weight: 800; color: #0F172A; background: transparent; border: none;");

    m_profileRoleBadge = new QLabel("🎓 STUDENT ACCOUNT", profileCard);
    m_profileRoleBadge->setStyleSheet(
        "font-size: 11px; font-weight: 700; color: #2563EB; background-color: #EFF6FF; padding: 4px 10px; border-radius: 6px; border: 1px solid #BFDBFE;"
    );

    nameRoleRow->addWidget(m_profileNameLabel);
    nameRoleRow->addWidget(m_profileRoleBadge);
    nameRoleRow->addStretch();

    m_profileAffiliationLabel = new QLabel("Graphic Era Hill University • Campus Recovery System", profileCard);
    m_profileAffiliationLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #64748B; background: transparent; border: none;");

    m_profileIdLabel = new QLabel("ID: #0 • student@gehu.ac.in", profileCard);
    m_profileIdLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #94A3B8; background: transparent; border: none;");

    profInfoLayout->addLayout(nameRoleRow);
    profInfoLayout->addWidget(m_profileAffiliationLabel);
    profInfoLayout->addWidget(m_profileIdLabel);

    profLayout->addLayout(profInfoLayout, 1);

    // Profile Action: Change Profile Image
    m_changeProfileBtn = new QPushButton("📷 Change Profile Image", profileCard);
    m_changeProfileBtn->setCursor(Qt::PointingHandCursor);
    m_changeProfileBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F8FAFC;"
        "   color: #1E293B;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 10px;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 12px 22px;"
        "   min-height: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EFF6FF;"
        "   border-color: #3B82F6;"
        "   color: #2563EB;"
        "}"
    );
    connect(m_changeProfileBtn, &QPushButton::clicked, this, &DashboardScreen::profileClicked);
    profLayout->addWidget(m_changeProfileBtn, 0, Qt::AlignVCenter);

    contentLayout->addWidget(profileCard);

    // =========================================================================
    // 3. USER ACTIVITY STATISTICS (YOUR ACTIVITY)
    // =========================================================================
    QVBoxLayout *actSectionLayout = new QVBoxLayout();
    actSectionLayout->setSpacing(14);

    QLabel *actTitle = new QLabel("YOUR ACTIVITY", contentWidget);
    actTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #64748B; letter-spacing: 1.5px; background: transparent;");
    actSectionLayout->addWidget(actTitle);

    QGridLayout *kpiGrid = new QGridLayout();
    kpiGrid->setSpacing(18);

    m_statTotalVal = new QLabel("0", contentWidget);
    m_statLostVal = new QLabel("0", contentWidget);
    m_statFoundVal = new QLabel("0", contentWidget);
    m_statResolvedVal = new QLabel("0", contentWidget);

    kpiGrid->addWidget(createKpiCard("📋", "TOTAL REPORTS", "All items reported by you", m_statTotalVal, "#2563EB"), 0, 0);
    kpiGrid->addWidget(createKpiCard("🔴", "LOST REPORTED", "Items currently marked lost", m_statLostVal, "#EF4444"), 0, 1);
    kpiGrid->addWidget(createKpiCard("🟢", "FOUND REPORTED", "Items reported as found", m_statFoundVal, "#10B981"), 0, 2);
    kpiGrid->addWidget(createKpiCard("✅", "RESOLVED / CLAIMED", "Successfully resolved items", m_statResolvedVal, "#8B5CF6"), 0, 3);

    actSectionLayout->addLayout(kpiGrid);
    contentLayout->addLayout(actSectionLayout);

    // =========================================================================
    // 4. QUICK ACTIONS
    // =========================================================================
    QVBoxLayout *qaSectionLayout = new QVBoxLayout();
    qaSectionLayout->setSpacing(14);

    QLabel *qaTitle = new QLabel("QUICK ACTIONS", contentWidget);
    qaTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #64748B; letter-spacing: 1.5px; background: transparent;");
    qaSectionLayout->addWidget(qaTitle);

    QHBoxLayout *qaCardsLayout = new QHBoxLayout();
    qaCardsLayout->setSpacing(18);

    qaCardsLayout->addWidget(createQuickActionCard(
        "📢",
        "Report Lost Item",
        "Report something you have lost on campus grounds to begin recovery.",
        "Report Lost Item →",
        "background-color: #EF4444; color: #FFFFFF;",
        2
    ));

    qaCardsLayout->addWidget(createQuickActionCard(
        "🎁",
        "Report Found Item",
        "Report an item you found on campus to help its rightful owner recover it.",
        "Report Found Item →",
        "background-color: #10B981; color: #FFFFFF;",
        2
    ));

    qaCardsLayout->addWidget(createQuickActionCard(
        "🔎",
        "Browse Items",
        "Search the campus lost & found registry for active listings and matches.",
        "Browse Items →",
        "background-color: #2563EB; color: #FFFFFF;",
        1
    ));

    qaSectionLayout->addLayout(qaCardsLayout);
    contentLayout->addLayout(qaSectionLayout);

    // =========================================================================
    // 5. MY CLAIMS SECTION
    // =========================================================================
    QVBoxLayout *claimsSectionLayout = new QVBoxLayout();
    claimsSectionLayout->setSpacing(14);

    QLabel *claimsTitle = new QLabel("MY CLAIMS", contentWidget);
    claimsTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #64748B; letter-spacing: 1.5px; background: transparent;");
    claimsSectionLayout->addWidget(claimsTitle);

    m_claimsCard = new QFrame(contentWidget);
    m_claimsCard->setObjectName("claimsCard");
    m_claimsCard->setStyleSheet(
        "QFrame#claimsCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 18px;"
        "}"
    );

    m_claimsCardLayout = new QVBoxLayout(m_claimsCard);
    m_claimsCardLayout->setContentsMargins(24, 22, 24, 22);
    m_claimsCardLayout->setSpacing(14);

    claimsSectionLayout->addWidget(m_claimsCard);
    contentLayout->addLayout(claimsSectionLayout);

    // =========================================================================
    // 6. MY RECENT REPORTS
    // =========================================================================
    QVBoxLayout *repSectionLayout = new QVBoxLayout();
    repSectionLayout->setSpacing(14);

    QHBoxLayout *repHeaderLayout = new QHBoxLayout();
    QLabel *repTitle = new QLabel("MY RECENT REPORTS", contentWidget);
    repTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #64748B; letter-spacing: 1.5px; background: transparent;");

    m_reportsCountBadge = new QLabel("0 reports", contentWidget);
    m_reportsCountBadge->setStyleSheet(
        "font-size: 11px; font-weight: 700; color: #2563EB; background-color: #EFF6FF; padding: 3px 10px; border-radius: 10px; border: 1px solid #BFDBFE;"
    );

    repHeaderLayout->addWidget(repTitle);
    repHeaderLayout->addSpacing(8);
    repHeaderLayout->addWidget(m_reportsCountBadge);
    repHeaderLayout->addStretch();
    repSectionLayout->addLayout(repHeaderLayout);

    m_reportsTableCard = new QFrame(contentWidget);
    m_reportsTableCard->setObjectName("reportsTableCard");
    m_reportsTableCard->setStyleSheet(
        "QFrame#reportsTableCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 18px;"
        "}"
    );

    m_reportsRowsLayout = new QVBoxLayout(m_reportsTableCard);
    m_reportsRowsLayout->setContentsMargins(0, 0, 0, 0);
    m_reportsRowsLayout->setSpacing(0);

    repSectionLayout->addWidget(m_reportsTableCard);
    contentLayout->addLayout(repSectionLayout);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea, 1);

    // Connect to ProfileImageManager for instant reactive updates when profile picture changes
    connect(&ProfileImageManager::instance(), &ProfileImageManager::profileImageChanged, this, [this](int userId) {
        if (AuthManager::instance().isLoggedIn() && AuthManager::instance().currentUser().id == userId) {
            updateProfile();
        }
    });
}

QFrame* DashboardScreen::createKpiCard(
    const QString &icon,
    const QString &label,
    const QString &supportingText,
    QLabel *valLabel,
    const QString &accentColor)
{
    QFrame *card = new QFrame();
    card->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "}"
        "QFrame:hover {"
        "   border-color: #CBD5E1;"
        "}"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 20, 22, 20);
    cardLayout->setSpacing(12);

    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    // Icon Circle
    QFrame *iconBox = new QFrame(card);
    iconBox->setFixedSize(48, 48);
    iconBox->setStyleSheet(QString(
        "QFrame {"
        "   background-color: %1;"
        "   border-radius: 24px;"
        "   border: none;"
        "}"
    ).arg(accentColor + "18")); // ~10% tint

    QHBoxLayout *ibLayout = new QHBoxLayout(iconBox);
    ibLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *iLbl = new QLabel(icon, iconBox);
    iLbl->setStyleSheet("font-size: 22px; background: transparent; border: none;");
    iLbl->setAlignment(Qt::AlignCenter);
    ibLayout->addWidget(iLbl);

    topRow->addWidget(iconBox);
    topRow->addStretch();
    cardLayout->addLayout(topRow);

    // Large Metric Number
    valLabel->setParent(card);
    valLabel->setStyleSheet("font-size: 34px; font-weight: 800; color: #0F172A; background: transparent; border: none;");
    cardLayout->addWidget(valLabel);

    // Metric Label
    QLabel *labelWidget = new QLabel(label, card);
    labelWidget->setStyleSheet("font-size: 13px; font-weight: 700; color: #475569; letter-spacing: 0.5px; background: transparent; border: none;");
    cardLayout->addWidget(labelWidget);

    // Supporting description
    QLabel *subWidget = new QLabel(supportingText, card);
    subWidget->setStyleSheet("font-size: 12px; font-weight: 500; color: #94A3B8; background: transparent; border: none;");
    subWidget->setWordWrap(true);
    cardLayout->addWidget(subWidget);

    return card;
}

QFrame* DashboardScreen::createQuickActionCard(
    const QString &icon,
    const QString &title,
    const QString &desc,
    const QString &btnText,
    const QString &btnStyle,
    int targetScreen)
{
    QFrame *card = new QFrame();
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "}"
        "QFrame:hover {"
        "   border-color: #CBD5E1;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);

    QHBoxLayout *headerRow = new QHBoxLayout();
    headerRow->setSpacing(10);

    QLabel *iconLabel = new QLabel(icon, card);
    iconLabel->setStyleSheet("font-size: 26px; background: transparent; border: none;");

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet("font-size: 17px; font-weight: 700; color: #0F172A; background: transparent; border: none;");

    headerRow->addWidget(iconLabel);
    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    layout->addLayout(headerRow);

    QLabel *descLabel = new QLabel(desc, card);
    descLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #64748B; background: transparent; border: none; line-height: 1.4;");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addSpacing(4);

    QPushButton *actionBtn = new QPushButton(btnText, card);
    actionBtn->setCursor(Qt::PointingHandCursor);
    actionBtn->setStyleSheet(QString(
        "QPushButton {"
        "   %1"
        "   font-size: 14px;"
        "   font-weight: 700;"
        "   padding: 12px 18px;"
        "   min-height: 20px;"
        "   border-radius: 10px;"
        "   border: none;"
        "}"
        "QPushButton:hover { opacity: 0.9; }"
    ).arg(btnStyle));

    connect(actionBtn, &QPushButton::clicked, this, [this, targetScreen]() {
        emit navigateTo(targetScreen);
    });

    layout->addWidget(actionBtn);

    return card;
}

QLabel* DashboardScreen::createStatusBadge(const QString &text, const QString &textColor, const QString &bgColor)
{
    QLabel *badge = new QLabel(text);
    badge->setStyleSheet(QString(
        "QLabel {"
        "   color: %1;"
        "   background-color: %2;"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   padding: 4px 10px;"
        "   border-radius: 6px;"
        "   border: none;"
        "}"
    ).arg(textColor, bgColor));
    badge->setAlignment(Qt::AlignCenter);
    return badge;
}

void DashboardScreen::updateProfile()
{
    User user = AuthManager::instance().currentUser();
    QString name = user.name.trimmed().isEmpty() ? "Student" : user.name.trimmed();

    m_greetingLabel->setText(QString("%1, %2 👋").arg(getGreeting(), name));

    if (m_profileNameLabel) {
        m_profileNameLabel->setText(name);
    }

    if (m_profileRoleBadge) {
        if (user.isAdmin()) {
            m_profileRoleBadge->setText("👑 ADMINISTRATOR");
            m_profileRoleBadge->setStyleSheet(
                "font-size: 11px; font-weight: 700; color: #7C3AED; background-color: #F5F3FF; padding: 4px 10px; border-radius: 6px; border: 1px solid #DDD6FE;"
            );
        } else {
            m_profileRoleBadge->setText("🎓 STUDENT ACCOUNT");
            m_profileRoleBadge->setStyleSheet(
                "font-size: 11px; font-weight: 700; color: #2563EB; background-color: #EFF6FF; padding: 4px 10px; border-radius: 6px; border: 1px solid #BFDBFE;"
            );
        }
    }

    if (m_profileIdLabel) {
        QString email = user.email.trimmed().isEmpty() ? "campus-user@gehu.ac.in" : user.email.trimmed();
        m_profileIdLabel->setText(QString("ID: #%1 • %2").arg(user.id).arg(email));
    }

    if (m_profileAvatarLabel) {
        QPixmap avatar = ProfileImageManager::instance().getCircularAvatar(user.id, name, 96, "#2563EB");
        m_profileAvatarLabel->setPixmap(avatar);
    }
}

void DashboardScreen::refreshData()
{
    updateProfile();

    // Query user reported items
    m_apiClient->getUserItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            renderUserItems(items);
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            Q_UNUSED(errorMessage);
            // Fallback to getAllItems filtered by user if getUserItems fails
            m_apiClient->getAllItems(
                [this](const QVector<Item> &items, int count) {
                    Q_UNUSED(count);
                    renderUserItems(items);
                },
                nullptr
            );
        }
    );

    // Query user claims
    m_apiClient->getUserClaims(
        [this](const QVector<Claim> &claims, int count) {
            Q_UNUSED(count);
            renderUserClaims(claims);
        },
        nullptr
    );
}

void DashboardScreen::renderUserClaims(const QVector<Claim> &claims)
{
    if (!m_claimsCardLayout) return;

    // Clear previous claims widgets safely
    QLayoutItem *child;
    while ((child = m_claimsCardLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    int pendingCount = 0;
    int approvedCount = 0;
    int handedOverCount = 0;
    int rejectedCount = 0;
    QString firstLoc = "Admin Block - Room 102";
    QString firstRef;

    for (const Claim &c : claims) {
        if (c.isApproved()) {
            approvedCount++;
            if (firstRef.isEmpty()) {
                firstRef = c.claimReference;
                firstLoc = c.pickupLocation.trimmed().isEmpty() ? "Admin Block - Room 102" : c.pickupLocation.trimmed();
            }
        } else if (c.isPending()) {
            pendingCount++;
        } else if (c.isHandedOver()) {
            handedOverCount++;
        } else if (c.isRejected()) {
            rejectedCount++;
        }
    }

    int activeTotal = pendingCount + approvedCount;

    if (claims.isEmpty()) {
        // Clean empty state
        QWidget *emptyWidget = new QWidget(m_claimsCard);
        QVBoxLayout *eLayout = new QVBoxLayout(emptyWidget);
        eLayout->setContentsMargins(16, 20, 16, 20);
        eLayout->setSpacing(10);
        eLayout->setAlignment(Qt::AlignCenter);

        QLabel *eIcon = new QLabel("🎒", emptyWidget);
        eIcon->setStyleSheet("font-size: 34px; background: transparent; border: none;");
        eIcon->setAlignment(Qt::AlignCenter);

        QLabel *eTitle = new QLabel("No active claims", emptyWidget);
        eTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #0F172A; background: transparent; border: none;");
        eTitle->setAlignment(Qt::AlignCenter);

        QLabel *eSub = new QLabel("You haven't submitted any claims yet. When you find your lost property in the catalog, submit a claim to verify ownership.", emptyWidget);
        eSub->setStyleSheet("font-size: 13px; font-weight: 500; color: #64748B; background: transparent; border: none;");
        eSub->setAlignment(Qt::AlignCenter);
        eSub->setWordWrap(true);

        QPushButton *browseBtn = new QPushButton("Browse Found Items →", emptyWidget);
        browseBtn->setCursor(Qt::PointingHandCursor);
        browseBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #2563EB;"
            "   color: #FFFFFF;"
            "   font-size: 13px;"
            "   font-weight: 700;"
            "   padding: 10px 22px;"
            "   border-radius: 8px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: #1D4ED8; }"
        );
        connect(browseBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); });

        eLayout->addWidget(eIcon);
        eLayout->addWidget(eTitle);
        eLayout->addWidget(eSub);
        eLayout->addSpacing(4);
        eLayout->addWidget(browseBtn, 0, Qt::AlignCenter);

        m_claimsCardLayout->addWidget(emptyWidget);
        return;
    }

    // Top Summary Row: Active Claims count + View My Claims button
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    QLabel *activeLabel = new QLabel(QString("Active Claims: %1").arg(activeTotal), m_claimsCard);
    activeLabel->setStyleSheet("font-size: 18px; font-weight: 800; color: #0F172A; background: transparent; border: none;");

    QPushButton *viewClaimsBtn = new QPushButton("View My Claims →", m_claimsCard);
    viewClaimsBtn->setCursor(Qt::PointingHandCursor);
    viewClaimsBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 9px 20px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );
    connect(viewClaimsBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(8); });

    topRow->addWidget(activeLabel);
    topRow->addStretch();
    topRow->addWidget(viewClaimsBtn);
    m_claimsCardLayout->addLayout(topRow);

    // Claims Status Metric Pills Row
    QHBoxLayout *pillsRow = new QHBoxLayout();
    pillsRow->setSpacing(12);

    QLabel *pendingPill = new QLabel(QString("🟡 Pending Review: %1").arg(pendingCount), m_claimsCard);
    pendingPill->setStyleSheet("font-size: 12px; font-weight: 700; color: #B45309; background-color: #FEF3C7; padding: 6px 14px; border-radius: 8px; border: 1px solid #FDE68A;");

    QLabel *approvedPill = new QLabel(QString("🟢 Ready for Pickup: %1").arg(approvedCount), m_claimsCard);
    approvedPill->setStyleSheet("font-size: 12px; font-weight: 700; color: #047857; background-color: #D1FAE5; padding: 6px 14px; border-radius: 8px; border: 1px solid #A7F3D0;");

    QLabel *handedOverPill = new QLabel(QString("🟣 Handed Over: %1").arg(handedOverCount), m_claimsCard);
    handedOverPill->setStyleSheet("font-size: 12px; font-weight: 700; color: #6D28D9; background-color: #EDE9FE; padding: 6px 14px; border-radius: 8px; border: 1px solid #DDD6FE;");

    QLabel *rejectedPill = new QLabel(QString("🔴 Rejected: %1").arg(rejectedCount), m_claimsCard);
    rejectedPill->setStyleSheet("font-size: 12px; font-weight: 700; color: #B91C1C; background-color: #FEE2E2; padding: 6px 14px; border-radius: 8px; border: 1px solid #FECDD3;");

    pillsRow->addWidget(pendingPill);
    pillsRow->addWidget(approvedPill);
    pillsRow->addWidget(handedOverPill);
    pillsRow->addWidget(rejectedPill);
    pillsRow->addStretch();
    m_claimsCardLayout->addLayout(pillsRow);

    // If there are approved items, show a prominent pickup pass notice
    if (approvedCount > 0) {
        QFrame *alertFrame = new QFrame(m_claimsCard);
        alertFrame->setStyleSheet(
            "QFrame {"
            "   background-color: #ECFDF5;"
            "   border: 1px solid #A7F3D0;"
            "   border-radius: 12px;"
            "   padding: 14px 18px;"
            "}"
        );
        QHBoxLayout *aLayout = new QHBoxLayout(alertFrame);
        aLayout->setContentsMargins(0, 0, 0, 0);
        aLayout->setSpacing(12);

        QLabel *aIcon = new QLabel("🎫", alertFrame);
        aIcon->setStyleSheet("font-size: 22px; background: transparent; border: none;");

        QLabel *aText = new QLabel(
            QString("<b>Action Required:</b> You have %1 approved item ready for physical collection at <b>%2</b> (Ref: %3)")
                .arg(approvedCount == 1 ? "1" : QString::number(approvedCount))
                .arg(firstLoc)
                .arg(firstRef),
            alertFrame
        );
        aText->setStyleSheet("font-size: 13px; color: #065F46; background: transparent; border: none;");

        QPushButton *passBtn = new QPushButton("View Pickup Pass →", alertFrame);
        passBtn->setCursor(Qt::PointingHandCursor);
        passBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #10B981;"
            "   color: #FFFFFF;"
            "   font-size: 12px;"
            "   font-weight: 700;"
            "   padding: 7px 16px;"
            "   border-radius: 6px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: #059669; }"
        );
        connect(passBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(8); });

        aLayout->addWidget(aIcon);
        aLayout->addWidget(aText, 1);
        aLayout->addWidget(passBtn);

        m_claimsCardLayout->addWidget(alertFrame);
    }
}

void DashboardScreen::renderUserItems(const QVector<Item> &items)
{
    int currentUserId = AuthManager::instance().currentUser().id;

    QVector<Item> userItems;
    int lostCount = 0;
    int foundCount = 0;
    int resolvedCount = 0;

    for (const Item &item : items) {
        if (item.userId == currentUserId) {
            userItems.append(item);
            if (item.status.toLower() == "claimed" || item.status.toLower() == "returned" || item.status.toLower() == "resolved") {
                resolvedCount++;
            } else if (item.isLost()) {
                lostCount++;
            } else {
                foundCount++;
            }
        }
    }

    m_statTotalVal->setText(QString::number(userItems.size()));
    m_statLostVal->setText(QString::number(lostCount));
    m_statFoundVal->setText(QString::number(foundCount));
    m_statResolvedVal->setText(QString::number(resolvedCount));

    if (m_reportsCountBadge) {
        m_reportsCountBadge->setText(QString("%1 %2").arg(userItems.size()).arg(userItems.size() == 1 ? "report" : "reports"));
    }

    // Clear previous reports rows safely
    QLayoutItem *child;
    while ((child = m_reportsRowsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    if (userItems.isEmpty()) {
        QWidget *emptyWidget = new QWidget(m_reportsTableCard);
        QVBoxLayout *eLayout = new QVBoxLayout(emptyWidget);
        eLayout->setContentsMargins(20, 36, 20, 36);
        eLayout->setSpacing(10);
        eLayout->setAlignment(Qt::AlignCenter);

        QLabel *eIcon = new QLabel("📋", emptyWidget);
        eIcon->setStyleSheet("font-size: 36px; background: transparent; border: none;");
        eIcon->setAlignment(Qt::AlignCenter);

        QLabel *eTitle = new QLabel("No reports yet", emptyWidget);
        eTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #0F172A; background: transparent; border: none;");
        eTitle->setAlignment(Qt::AlignCenter);

        QLabel *eSub = new QLabel("Your reported lost and found items will appear here.", emptyWidget);
        eSub->setStyleSheet("font-size: 13px; font-weight: 500; color: #64748B; background: transparent; border: none;");
        eSub->setAlignment(Qt::AlignCenter);

        QPushButton *repBtn = new QPushButton("+ Report an Item", emptyWidget);
        repBtn->setCursor(Qt::PointingHandCursor);
        repBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #2563EB;"
            "   color: #FFFFFF;"
            "   font-size: 13px;"
            "   font-weight: 700;"
            "   padding: 10px 22px;"
            "   border-radius: 8px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: #1D4ED8; }"
        );
        connect(repBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });

        eLayout->addWidget(eIcon);
        eLayout->addWidget(eTitle);
        eLayout->addWidget(eSub);
        eLayout->addSpacing(4);
        eLayout->addWidget(repBtn, 0, Qt::AlignCenter);

        m_reportsRowsLayout->addWidget(emptyWidget);
        return;
    }

    // Add Table Header Row
    QFrame *thRow = new QFrame(m_reportsTableCard);
    thRow->setStyleSheet(
        "QFrame {"
        "   background-color: #F8FAFC;"
        "   border-bottom: 1px solid #E2E8F0;"
        "   border-top-left-radius: 18px;"
        "   border-top-right-radius: 18px;"
        "}"
        "QLabel {"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   color: #64748B;"
        "   letter-spacing: 0.8px;"
        "   background: transparent;"
        "   border: none;"
        "}"
    );

    QHBoxLayout *thLayout = new QHBoxLayout(thRow);
    thLayout->setContentsMargins(24, 14, 24, 14);
    thLayout->setSpacing(16);

    QLabel *thItem = new QLabel("ITEM", thRow);
    QLabel *thType = new QLabel("TYPE", thRow);
    QLabel *thStatus = new QLabel("STATUS", thRow);
    QLabel *thDate = new QLabel("DATE REPORTED", thRow);
    QLabel *thAction = new QLabel("ACTION", thRow);
    thAction->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    thLayout->addWidget(thItem, 4);
    thLayout->addWidget(thType, 2);
    thLayout->addWidget(thStatus, 2);
    thLayout->addWidget(thDate, 2);
    thLayout->addWidget(thAction, 2);

    m_reportsRowsLayout->addWidget(thRow);

    // Render each item report row
    for (int i = 0; i < userItems.size(); ++i) {
        QWidget *row = createRecentReportRow(userItems[i]);
        m_reportsRowsLayout->addWidget(row);
    }
}

QWidget* DashboardScreen::createRecentReportRow(const Item &item)
{
    QFrame *row = new QFrame(m_reportsTableCard);
    row->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #F1F5F9;"
        "}"
        "QFrame:hover {"
        "   background-color: #F8FBFF;"
        "}"
    );

    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(24, 14, 24, 14);
    layout->setSpacing(16);

    // 1. Item Column: Thumbnail + Title + Location
    QHBoxLayout *itemCol = new QHBoxLayout();
    itemCol->setSpacing(12);

    QLabel *thumbLabel = new QLabel(row);
    thumbLabel->setFixedSize(44, 44);
    thumbLabel->setStyleSheet("background-color: #F1F5F9; border-radius: 8px; border: 1px solid #E2E8F0;");
    thumbLabel->setAlignment(Qt::AlignCenter);

    if (item.hasImage()) {
        ImageLoader::instance().loadIntoLabel(item.imageUrl(), thumbLabel, QSize(44, 44));
    } else {
        thumbLabel->setText(item.isLost() ? "🔍" : "📦");
        thumbLabel->setStyleSheet("background-color: #F8FAFC; border-radius: 8px; border: 1px solid #E2E8F0; font-size: 20px;");
    }

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    titleLayout->setAlignment(Qt::AlignVCenter);

    QLabel *titleLabel = new QLabel(item.title, row);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #0F172A; background: transparent; border: none;");

    QLabel *locLabel = new QLabel(QString("📍 %1").arg(item.location.isEmpty() ? "Campus" : item.location), row);
    locLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #64748B; background: transparent; border: none;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(locLabel);

    itemCol->addWidget(thumbLabel);
    itemCol->addLayout(titleLayout);
    itemCol->addStretch();

    // 2. Type Column
    QLabel *typeBadge;
    if (item.isLost()) {
        typeBadge = createStatusBadge("LOST", "#DC2626", "#FEE2E2");
    } else {
        typeBadge = createStatusBadge("FOUND", "#16A34A", "#DCFCE7");
    }

    // 3. Status Column
    QLabel *statusBadge;
    QString st = item.status.toLower();
    if (st == "claimed" || st == "returned" || st == "resolved") {
        statusBadge = createStatusBadge("RESOLVED", "#15803D", "#DCFCE7");
    } else if (st == "pending") {
        statusBadge = createStatusBadge("PENDING", "#D97706", "#FEF3C7");
    } else {
        statusBadge = createStatusBadge("ACTIVE", "#1D4ED8", "#EFF6FF");
    }

    // 4. Date Column
    QString dateStr = item.createdAt;
    if (dateStr.contains("T")) {
        dateStr = dateStr.split("T").first();
    } else if (dateStr.contains(" ")) {
        dateStr = dateStr.split(" ").first();
    }
    if (dateStr.isEmpty()) dateStr = "Recent";

    QLabel *dateLabel = new QLabel(dateStr, row);
    dateLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #64748B; background: transparent; border: none;");

    // 5. Action Column
    QHBoxLayout *actionCol = new QHBoxLayout();
    actionCol->setSpacing(8);
    actionCol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QPushButton *viewBtn = new QPushButton("View Details →", row);
    viewBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #EFF6FF;"
        "   color: #2563EB;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 6px 14px;"
        "   border-radius: 6px;"
        "   border: 1px solid #BFDBFE;"
        "}"
        "QPushButton:hover {"
        "   background-color: #DBEAFE;"
        "}"
    );
    connect(viewBtn, &QPushButton::clicked, this, [this, item]() {
        emit viewItemDetails(item);
    });

    QPushButton *delBtn = new QPushButton("🗑️", row);
    delBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setToolTip("Delete this report");
    delBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #EF4444;"
        "   font-size: 13px;"
        "   padding: 6px 10px;"
        "   border-radius: 6px;"
        "   border: 1px solid #FECDD3;"
        "}"
        "QPushButton:hover {"
        "   background-color: #FEE2E2;"
        "}"
    );
    connect(delBtn, &QPushButton::clicked, this, [this, item]() {
        confirmDeleteItem(item);
    });

    actionCol->addWidget(viewBtn);
    actionCol->addWidget(delBtn);

    layout->addLayout(itemCol, 4);
    layout->addWidget(typeBadge, 2, Qt::AlignLeft);
    layout->addWidget(statusBadge, 2, Qt::AlignLeft);
    layout->addWidget(dateLabel, 2, Qt::AlignLeft);
    layout->addLayout(actionCol, 2);

    return row;
}

void DashboardScreen::confirmDeleteItem(const Item &item)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Item Report",
        QString("Are you sure you want to delete \"%1\"?\nThis cannot be undone.").arg(item.title),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    m_apiClient->deleteItem(
        item.id,
        [this, item](const QJsonObject &response) {
            Q_UNUSED(response);
            emit showToast(QString("Item \"%1\" deleted successfully.").arg(item.title), ToastType::Success);
            refreshData();
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            QMessageBox::critical(
                this,
                "Delete Failed",
                QString("Unable to delete item: %1").arg(errorMessage)
            );
        }
    );
}