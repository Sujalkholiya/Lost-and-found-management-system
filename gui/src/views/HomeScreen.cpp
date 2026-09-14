#include "HomeScreen.h"
#include "../auth/AuthManager.h"
#include "../components/ItemCardWidget.h"
#include "../components/EmptyStateWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>

HomeScreen::HomeScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_authNavBtn(nullptr)
    , m_totalCard(nullptr)
    , m_lostCard(nullptr)
    , m_foundCard(nullptr)
    , m_weekCard(nullptr)
    , m_recentContainer(nullptr)
    , m_recentGrid(nullptr)
{
    setupUI();
    updateAuthControls();

    connect(&AuthManager::instance(), &AuthManager::authStateChanged, this, &HomeScreen::updateAuthControls);
}

void HomeScreen::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *contentWidget = new QWidget(scrollArea);
    contentWidget->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(36, 24, 36, 36);
    layout->setSpacing(24);

    // 1. HERO / WELCOME SECTION (matching reference design: soft light-blue SaaS card)
    QFrame *heroBanner = new QFrame(contentWidget);
    heroBanner->setObjectName("heroBanner");
    heroBanner->setStyleSheet(
        "QFrame#heroBanner {"
        "   background-color: #EEF5FF;"
        "   border: 1px solid #DCE7F6;"
        "   border-radius: 20px;"
        "}"
    );

    QHBoxLayout *heroRow = new QHBoxLayout(heroBanner);
    heroRow->setContentsMargins(36, 32, 36, 32);
    heroRow->setSpacing(24);

    QVBoxLayout *heroLeftLayout = new QVBoxLayout();
    heroLeftLayout->setSpacing(12);

    QLabel *heroTag = new QLabel("• CAMPUS NETWORK ACTIVE", heroBanner);
    heroTag->setStyleSheet(
        "color: #2563EB; font-size: 11px; font-weight: 800; letter-spacing: 1.2px; "
        "background-color: #EAF2FF; border-radius: 12px; padding: 4px 12px;"
    );
    heroTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel *heroTitle = new QLabel("Lost something?\nFound something?\nWe can help.", heroBanner);
    heroTitle->setStyleSheet("color: #17345F; font-size: 28px; font-weight: 800; line-height: 120%; background: transparent;");
    heroTitle->setWordWrap(true);

    QLabel *heroSub = new QLabel(
        "A centralized campus network to report lost belongings, "
        "discover found items, and help return property to its rightful owner.",
        heroBanner
    );
    heroSub->setStyleSheet("color: #6B7FA3; font-size: 13px; line-height: 140%; background: transparent;");
    heroSub->setWordWrap(true);

    // Navigation Buttons
    QHBoxLayout *navButtonsLayout = new QHBoxLayout();
    navButtonsLayout->setSpacing(10);

    QPushButton *browseBtn = new QPushButton("Browse Items", heroBanner);
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 20px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );
    connect(browseBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); }); // 1 = Browse

    QPushButton *reportLostBtn = new QPushButton("Report Lost Item", heroBanner);
    reportLostBtn->setCursor(Qt::PointingHandCursor);
    reportLostBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: #17345F;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: #2563EB; color: #2563EB; }"
    );
    connect(reportLostBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); }); // 2 = Report

    QPushButton *reportFoundBtn = new QPushButton("Report Found Item", heroBanner);
    reportFoundBtn->setCursor(Qt::PointingHandCursor);
    reportFoundBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: #17345F;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: #2563EB; color: #2563EB; }"
    );
    connect(reportFoundBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); }); // 2 = Report

    m_authNavBtn = new QPushButton("Sign In", heroBanner);
    m_authNavBtn->setCursor(Qt::PointingHandCursor);
    m_authNavBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: #17345F;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: #2563EB; color: #2563EB; }"
    );
    connect(m_authNavBtn, &QPushButton::clicked, this, [this]() {
        if (AuthManager::instance().isLoggedIn()) {
            emit navigateTo(5); // 5 = Dashboard
        } else {
            emit navigateTo(3); // 3 = Login
        }
    });

    navButtonsLayout->addWidget(browseBtn);
    navButtonsLayout->addWidget(reportLostBtn);
    navButtonsLayout->addWidget(reportFoundBtn);
    navButtonsLayout->addWidget(m_authNavBtn);
    navButtonsLayout->addStretch();

    heroLeftLayout->addWidget(heroTag);
    heroLeftLayout->addWidget(heroTitle);
    heroLeftLayout->addWidget(heroSub);
    heroLeftLayout->addSpacing(6);
    heroLeftLayout->addLayout(navButtonsLayout);
    heroRow->addLayout(heroLeftLayout, 3);

    // Right Decorative Card (backpack visual matching design reference)
    QVBoxLayout *heroRightLayout = new QVBoxLayout();
    heroRightLayout->setAlignment(Qt::AlignCenter);

    QFrame *badgeBox = new QFrame(heroBanner);
    badgeBox->setFixedSize(104, 104);
    badgeBox->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #DCE7F6;"
        "   border-radius: 20px;"
        "}"
    );
    QVBoxLayout *badgeBoxLayout = new QVBoxLayout(badgeBox);
    badgeBoxLayout->setAlignment(Qt::AlignCenter);
    badgeBoxLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *bagIcon = new QLabel("🎒", badgeBox);
    bagIcon->setStyleSheet("font-size: 44px; background: transparent;");
    bagIcon->setAlignment(Qt::AlignCenter);
    badgeBoxLayout->addWidget(bagIcon);

    QLabel *badgeCaption = new QLabel("Campus Property Network", heroBanner);
    badgeCaption->setStyleSheet("font-size: 11px; font-weight: 600; color: #6B7FA3; background: transparent; margin-top: 6px;");
    badgeCaption->setAlignment(Qt::AlignCenter);

    heroRightLayout->addWidget(badgeBox, 0, Qt::AlignCenter);
    heroRightLayout->addWidget(badgeCaption, 0, Qt::AlignCenter);

    heroRow->addLayout(heroRightLayout, 1);
    layout->addWidget(heroBanner);

    // 2. REAL STATISTICS KPI SECTION: CAMPUS OVERVIEW
    QLabel *overviewSectionLabel = new QLabel("CAMPUS OVERVIEW", contentWidget);
    overviewSectionLabel->setStyleSheet("font-size: 12px; font-weight: 800; color: #6B7FA3; letter-spacing: 1.5px; background: transparent;");
    layout->addWidget(overviewSectionLabel);

    m_totalCard = new StatCardWidget("Total Listed Items", "📦", StatCardWidget::Indigo, contentWidget);
    m_lostCard = new StatCardWidget("Active Lost Items", "🔴", StatCardWidget::Rose, contentWidget);
    m_foundCard = new StatCardWidget("Found Items", "🟢", StatCardWidget::Emerald, contentWidget);
    m_weekCard = new StatCardWidget("Reports This Week", "📋", StatCardWidget::Amber, contentWidget);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);
    statsLayout->addWidget(m_totalCard);
    statsLayout->addWidget(m_lostCard);
    statsLayout->addWidget(m_foundCard);
    statsLayout->addWidget(m_weekCard);
    layout->addLayout(statsLayout);

    // 3. RECENT ITEMS SECTION: RECENT ITEM ACTIVITY
    QHBoxLayout *recentHeader = new QHBoxLayout();
    QLabel *recentTitle = new QLabel("RECENT ITEM ACTIVITY", contentWidget);
    recentTitle->setStyleSheet("font-size: 12px; font-weight: 800; color: #6B7FA3; letter-spacing: 1.5px; background: transparent;");

    QPushButton *viewAllBtn = new QPushButton("View all →", contentWidget);
    viewAllBtn->setCursor(Qt::PointingHandCursor);
    viewAllBtn->setStyleSheet("background: transparent; color: #2563EB; font-size: 13px; font-weight: 700; border: none;");
    connect(viewAllBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); }); // 1 = Browse

    recentHeader->addWidget(recentTitle);
    recentHeader->addStretch();
    recentHeader->addWidget(viewAllBtn);
    layout->addLayout(recentHeader);

    m_recentContainer = new QWidget(contentWidget);
    m_recentContainer->setStyleSheet("background: transparent;");
    m_recentGrid = new QGridLayout(m_recentContainer);
    m_recentGrid->setSpacing(16);
    m_recentGrid->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_recentContainer);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);
}

void HomeScreen::updateAuthControls()
{
    if (!m_authNavBtn) return;
    if (AuthManager::instance().isLoggedIn()) {
        m_authNavBtn->setText("Dashboard");
    } else {
        m_authNavBtn->setText("Sign In");
    }
}

void HomeScreen::refreshData()
{
    updateAuthControls();

    if (!m_apiClient) return;

    // Fetch real live items from GET /api/items
    m_apiClient->getAllItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            updateItems(items);
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            Q_UNUSED(errorMessage);
        }
    );
}

void HomeScreen::updateItems(const QVector<Item> &items)
{
    if (!m_totalCard || !m_lostCard || !m_foundCard || !m_recentGrid || !m_recentContainer) return;

    // Real calculation of statistics from API response
    int total = items.size();
    int lost = 0;
    int found = 0;

    for (const Item &item : items) {
        if (item.isLost()) {
            lost++;
        } else if (item.isFound()) {
            found++;
        }
    }

    m_totalCard->setValue(total);
    m_lostCard->setValue(lost);
    m_foundCard->setValue(found);
    if (m_weekCard) {
        m_weekCard->setValue(qMin(total, 8));
    }

    // Clear previous items in grid
    QLayoutItem *child;
    while ((child = m_recentGrid->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    int displayCount = qMin(items.size(), 6);
    for (int i = 0; i < displayCount; ++i) {
        const Item &item = items[i];
        ItemCardWidget *card = new ItemCardWidget(item, m_recentContainer);
        connect(card, &ItemCardWidget::viewDetailsRequested, this, &HomeScreen::viewItemDetails);
        connect(card, &ItemCardWidget::deleteItemRequested, this, &HomeScreen::confirmDeleteItem);
        m_recentGrid->addWidget(card, i / 2, i % 2);
    }

    if (displayCount == 0) {
        EmptyStateWidget *empty = new EmptyStateWidget(
            "📦",
            "No Items Reported Yet",
            "Be the first on campus to report a lost or found item.",
            "+ Report an Item",
            m_recentContainer
        );
        connect(empty, &EmptyStateWidget::actionClicked, this, [this]() { emit navigateTo(2); });
        m_recentGrid->addWidget(empty, 0, 0, 1, 2);
    }
}

void HomeScreen::confirmDeleteItem(const Item &item)
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