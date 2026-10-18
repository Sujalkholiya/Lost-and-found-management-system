#include "AdminScreen.h"
#include "AdminClaimReviewDialog.h"
#include "../auth/AuthManager.h"
#include "../components/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QFrame>
#include <QShowEvent>
#include <QHideEvent>
#include <QDialog>
#include <QMessageBox>
#include <QDebug>

AdminScreen::AdminScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_currentClaimsFilter("all")
    , m_roleBadgeLabel(nullptr)
    , m_pendingBadgeLabel(nullptr)
    , m_tabWidget(nullptr)
    , m_totalCard(nullptr)
    , m_lostCard(nullptr)
    , m_foundCard(nullptr)
    , m_activeCard(nullptr)
    , m_pendingClaimsCard(nullptr)
    , m_approvedClaimsCard(nullptr)
    , m_handedOverClaimsCard(nullptr)
    , m_rejectedClaimsCard(nullptr)
    , m_searchEdit(nullptr)
    , m_claimsSearchEdit(nullptr)
    , m_refreshBtn(nullptr)
    , m_recordCountLabel(nullptr)
    , m_claimsCountLabel(nullptr)
    , m_cBtnAll(nullptr)
    , m_cBtnPending(nullptr)
    , m_cBtnApproved(nullptr)
    , m_cBtnHandedOver(nullptr)
    , m_cBtnRejected(nullptr)
    , m_forbiddenBox(nullptr)
    , m_contentBox(nullptr)
    , m_loadingBox(nullptr)
    , m_progressBar(nullptr)
    , m_errorBox(nullptr)
    , m_errorLabel(nullptr)
    , m_retryBtn(nullptr)
    , m_table(nullptr)
    , m_claimsTable(nullptr)
    , m_pollTimer(nullptr)
    , m_isFirstPoll(true)
    , m_isPolling(false)
{
    setupUI();

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(3000);
    connect(m_pollTimer, &QTimer::timeout, this, &AdminScreen::pollPendingClaims);

    connect(&AuthManager::instance(), &AuthManager::authStateChanged, this, [this](bool loggedIn) {
        Q_UNUSED(loggedIn);
        checkPermissions();
    });
}

void AdminScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    checkPermissions();
    if (AuthManager::instance().isAdmin()) {
        // Open Admin Console with Claim Management selected by default
        if (m_tabWidget && m_tabWidget->currentIndex() != 0) {
            m_tabWidget->setCurrentIndex(0);
        }
        if (m_pollTimer && !m_pollTimer->isActive()) {
            m_pollTimer->start(3000);
        }
    }
}

void AdminScreen::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (m_pollTimer && m_pollTimer->isActive()) {
        m_pollTimer->stop();
    }
}

void AdminScreen::switchToTab(int tabIndex)
{
    if (m_tabWidget && tabIndex >= 0 && tabIndex < m_tabWidget->count()) {
        m_tabWidget->setCurrentIndex(tabIndex);
    }
}

void AdminScreen::checkPermissions()
{
    bool isAdmin = AuthManager::instance().isAdmin();
    m_forbiddenBox->setVisible(!isAdmin);
    m_contentBox->setVisible(isAdmin);

    if (isAdmin) {
        refreshData();
        if (m_pollTimer && !m_pollTimer->isActive() && isVisible()) {
            m_pollTimer->start(3000);
        }
    } else {
        if (m_pollTimer && m_pollTimer->isActive()) {
            m_pollTimer->stop();
        }
        m_isFirstPoll = true;
        m_knownPendingClaimIds.clear();
    }
}

void AdminScreen::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(36, 28, 36, 36);
    rootLayout->setSpacing(20);

    // Header & Top Row
    QHBoxLayout *headerRow = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(4);

    QHBoxLayout *titleBadgeRow = new QHBoxLayout();
    titleBadgeRow->setSpacing(10);

    QLabel *pageTitle = new QLabel("🛡️ Campus Administration Console", this);
    pageTitle->setStyleSheet("font-size: 24px; font-weight: 700; color: " + Theme::Text::Primary + "; background: transparent;");
    titleBadgeRow->addWidget(pageTitle);

    m_roleBadgeLabel = new QLabel("👑 Administrator", this);
    m_roleBadgeLabel->setStyleSheet(
        "background-color: #FEF3C7; color: #92400E; font-size: 12px; font-weight: 700; padding: 4px 10px; border-radius: 12px; border: 1px solid #FDE68A;"
    );
    titleBadgeRow->addWidget(m_roleBadgeLabel);

    m_pendingBadgeLabel = new QLabel("🔔 0 Pending", this);
    m_pendingBadgeLabel->setStyleSheet(
        "background-color: #FEE2E2; color: #B91C1C; font-size: 12px; font-weight: 700; padding: 4px 10px; border-radius: 12px; border: 1px solid #FECDD3;"
    );
    titleBadgeRow->addWidget(m_pendingBadgeLabel);
    titleBadgeRow->addStretch();

    QLabel *pageSub = new QLabel("Restricted administrative console for campus property verification, claims approval, and master inventory audit.", this);
    pageSub->setStyleSheet("font-size: 13px; color: " + Theme::Text::Secondary + "; background: transparent;");

    titleLayout->addLayout(titleBadgeRow);
    titleLayout->addWidget(pageSub);
    headerRow->addLayout(titleLayout);
    headerRow->addStretch();

    m_refreshBtn = new QPushButton("🔄 Refresh Data", this);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: " + Theme::Primary::Main + ";"
        "   color: white;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "   padding: 10px 18px;"
        "   border-radius: 10px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: " + Theme::Primary::Dark + "; }"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, &AdminScreen::refreshData);
    headerRow->addWidget(m_refreshBtn);

    rootLayout->addLayout(headerRow);

    // 1. FORBIDDEN ACCESS BARRIER CARD (Shown if non-admin or unauthenticated)
    m_forbiddenBox = new QFrame(this);
    m_forbiddenBox->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 18px;"
        "   padding: 48px 32px;"
        "}"
    );

    QVBoxLayout *forbidLayout = new QVBoxLayout(m_forbiddenBox);
    forbidLayout->setAlignment(Qt::AlignCenter);
    forbidLayout->setSpacing(16);

    QLabel *shieldIcon = new QLabel("🔒", m_forbiddenBox);
    shieldIcon->setStyleSheet("font-size: 56px; background: transparent;");
    shieldIcon->setAlignment(Qt::AlignCenter);

    QLabel *forbidTitle = new QLabel("Restricted Administrator Access", m_forbiddenBox);
    forbidTitle->setStyleSheet("font-size: 20px; font-weight: 700; color: " + Theme::Text::Primary + "; background: transparent;");
    forbidTitle->setAlignment(Qt::AlignCenter);

    QLabel *forbidMsg = new QLabel(
        "You do not possess the necessary administrative privileges to view this console.\n"
        "Please log in with a verified staff or administrator university account.",
        m_forbiddenBox
    );
    forbidMsg->setStyleSheet("font-size: 14px; color: " + Theme::Text::Secondary + "; background: transparent; line-height: 140%;");
    forbidMsg->setAlignment(Qt::AlignCenter);

    QPushButton *toLoginBtn = new QPushButton("Authenticate as Administrator", m_forbiddenBox);
    toLoginBtn->setCursor(Qt::PointingHandCursor);
    toLoginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: " + Theme::Primary::Main + ";"
        "   color: white;"
        "   font-weight: 600;"
        "   font-size: 14px;"
        "   padding: 12px 24px;"
        "   border-radius: 10px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: " + Theme::Primary::Dark + "; }"
    );
    connect(toLoginBtn, &QPushButton::clicked, this, [this]() {
        emit navigateTo(4); // Login screen index
    });

    forbidLayout->addWidget(shieldIcon);
    forbidLayout->addWidget(forbidTitle);
    forbidLayout->addWidget(forbidMsg);
    forbidLayout->addWidget(toLoginBtn, 0, Qt::AlignCenter);
    m_forbiddenBox->hide();
    rootLayout->addWidget(m_forbiddenBox);

    // 2. AUTHORIZED CONTENT CONTAINER
    m_contentBox = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentBox);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(16);

    // Loading State Box
    m_loadingBox = new QWidget(m_contentBox);
    QVBoxLayout *loadLayout = new QVBoxLayout(m_loadingBox);
    loadLayout->setContentsMargins(0, 8, 0, 8);
    loadLayout->setSpacing(8);

    QLabel *loadLabel = new QLabel("⏳ Querying admin console data from backend server...", m_loadingBox);
    loadLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: " + Theme::Primary::Main + ";");
    loadLabel->setAlignment(Qt::AlignCenter);

    m_progressBar = new QProgressBar(m_loadingBox);
    m_progressBar->setRange(0, 0);
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: " + Theme::Background::Page + "; border-radius: 2px; border: none; }"
        "QProgressBar::chunk { background-color: " + Theme::Primary::Main + "; border-radius: 2px; }"
    );

    loadLayout->addWidget(loadLabel);
    loadLayout->addWidget(m_progressBar);
    m_loadingBox->hide();
    contentLayout->addWidget(m_loadingBox);

    // Error State Box
    m_errorBox = new QWidget(m_contentBox);
    m_errorBox->setStyleSheet(
        "QWidget {"
        "   background-color: #FEF2F2;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 10px;"
        "   padding: 14px;"
        "}"
    );
    QHBoxLayout *errorLayout = new QHBoxLayout(m_errorBox);
    m_errorLabel = new QLabel(m_errorBox);
    m_errorLabel->setStyleSheet("color: " + Theme::Status::Error + "; font-size: 13px; font-weight: 600; background: transparent;");

    m_retryBtn = new QPushButton("Retry Request", m_errorBox);
    m_retryBtn->setCursor(Qt::PointingHandCursor);
    m_retryBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: " + Theme::Status::Error + ";"
        "   color: white;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "   padding: 6px 14px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #B91C1C; }"
    );
    connect(m_retryBtn, &QPushButton::clicked, this, &AdminScreen::refreshData);

    errorLayout->addWidget(m_errorLabel);
    errorLayout->addStretch();
    errorLayout->addWidget(m_retryBtn);
    m_errorBox->hide();
    contentLayout->addWidget(m_errorBox);

    // TAB WIDGET
    m_tabWidget = new QTabWidget(m_contentBox);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "   border: none;"
        "   background: transparent;"
        "}"
        "QTabBar::tab {"
        "   background: #FFFFFF;"
        "   color: " + Theme::Text::Secondary + ";"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 11px 24px;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-bottom: none;"
        "   border-top-left-radius: 10px;"
        "   border-top-right-radius: 10px;"
        "   margin-right: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "   background: #FFFFFF;"
        "   color: " + Theme::Primary::Main + ";"
        "   border-top: 3px solid " + Theme::Primary::Main + ";"
        "   font-weight: 700;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "   background: " + Theme::Primary::Subtle + ";"
        "   color: " + Theme::Primary::Main + ";"
        "}"
    );
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &AdminScreen::tabChanged);

    // ==========================================
    // TAB 0: CLAIM MANAGEMENT (Primary Admin Tab)
    // ==========================================
    QWidget *claimsTab = new QWidget();
    QVBoxLayout *claimsTabLayout = new QVBoxLayout(claimsTab);
    claimsTabLayout->setContentsMargins(0, 16, 0, 0);
    claimsTabLayout->setSpacing(16);

    // 4 Claims KPI Cards: Pending Review, Approved / Ready for Pickup, Handed Over, Rejected
    QHBoxLayout *claimsStatsRow = new QHBoxLayout();
    claimsStatsRow->setSpacing(14);
    m_pendingClaimsCard = new StatCardWidget("Pending Review", "⏳", StatCardWidget::Amber, claimsTab);
    m_approvedClaimsCard = new StatCardWidget("Approved / Ready for Pickup", "🎫", StatCardWidget::Emerald, claimsTab);
    m_handedOverClaimsCard = new StatCardWidget("Handed Over", "🤝", StatCardWidget::Indigo, claimsTab);
    m_rejectedClaimsCard = new StatCardWidget("Rejected", "❌", StatCardWidget::Rose, claimsTab);

    claimsStatsRow->addWidget(m_pendingClaimsCard);
    claimsStatsRow->addWidget(m_approvedClaimsCard);
    claimsStatsRow->addWidget(m_handedOverClaimsCard);
    claimsStatsRow->addWidget(m_rejectedClaimsCard);
    claimsTabLayout->addLayout(claimsStatsRow);

    // Claims Frame Card
    QFrame *claimsCard = new QFrame(claimsTab);
    claimsCard->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 16px;"
        "   padding: 18px;"
        "}"
    );
    QVBoxLayout *claimsCardLayout = new QVBoxLayout(claimsCard);
    claimsCardLayout->setSpacing(12);

    // Claims Filter Buttons & Search Bar
    QHBoxLayout *cFilterRow = new QHBoxLayout();
    cFilterRow->setSpacing(8);

    auto makeClaimFilterBtn = [this](const QString& text, const QString& filter) {
        QPushButton *btn = new QPushButton(text);
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, filter]() {
            onClaimFilterBtnClicked(filter);
        });
        return btn;
    };

    m_cBtnAll = makeClaimFilterBtn("All", "all");
    m_cBtnPending = makeClaimFilterBtn("⏳ Pending Review", "pending");
    m_cBtnApproved = makeClaimFilterBtn("🎫 Ready for Pickup", "approved");
    m_cBtnHandedOver = makeClaimFilterBtn("🤝 Handed Over", "handed_over");
    m_cBtnRejected = makeClaimFilterBtn("❌ Rejected", "rejected");

    cFilterRow->addWidget(m_cBtnAll);
    cFilterRow->addWidget(m_cBtnPending);
    cFilterRow->addWidget(m_cBtnApproved);
    cFilterRow->addWidget(m_cBtnHandedOver);
    cFilterRow->addWidget(m_cBtnRejected);
    cFilterRow->addStretch();

    m_claimsCountLabel = new QLabel("0 claims", claimsCard);
    m_claimsCountLabel->setStyleSheet("font-size: 12px; color: " + Theme::Text::Secondary + "; font-weight: 600;");
    cFilterRow->addWidget(m_claimsCountLabel);

    m_claimsSearchEdit = new QLineEdit(claimsCard);
    m_claimsSearchEdit->setPlaceholderText("🔍 Search ref, claimant, item...");
    m_claimsSearchEdit->setFixedWidth(240);
    m_claimsSearchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 10px;"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   color: " + Theme::Text::Primary + ";"
        "}"
        "QLineEdit:focus { border: 2px solid " + Theme::Primary::Main + "; background-color: #FFFFFF; }"
    );
    connect(m_claimsSearchEdit, &QLineEdit::textChanged, this, &AdminScreen::applyClaimsFilter);
    cFilterRow->addWidget(m_claimsSearchEdit);

    claimsCardLayout->addLayout(cFilterRow);

    // Claims Table Widget: 7 Columns
    // Reference | Target Item | Claimant | Claim Date | Status | Pickup Station | Actions
    m_claimsTable = new QTableWidget(claimsCard);
    m_claimsTable->setColumnCount(7);

    QStringList headers;
    headers << "Reference"
            << "Target Item"
            << "Claimant"
            << "Claim Date"
            << "Status"
            << "Pickup Station"
            << "Actions";
    m_claimsTable->setHorizontalHeaderLabels(headers);

    m_claimsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    m_claimsTable->setColumnWidth(6, 180);
    m_claimsTable->horizontalHeader()->setStretchLastSection(false);
    m_claimsTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_claimsTable->verticalHeader()->setDefaultSectionSize(48);

    m_claimsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_claimsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_claimsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_claimsTable->setShowGrid(true);
    m_claimsTable->setAlternatingRowColors(true);
    m_claimsTable->setMinimumHeight(380);

    m_claimsTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #FFFFFF;"
        "   alternate-background-color: #FAFCFE;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 10px;"
        "   font-size: 13px;"
        "   gridline-color: #F1F5F9;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: " + Theme::Text::Secondary + ";"
        "   font-weight: 700;"
        "   border: none;"
        "   border-bottom: 2px solid " + Theme::Border::Default + ";"
        "   padding: 10px 8px;"
        "   font-size: 11px;"
        "   text-transform: uppercase;"
        "   letter-spacing: 0.5px;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px 6px;"
        "   color: " + Theme::Text::Primary + ";"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: " + Theme::Primary::Subtle + ";"
        "   color: " + Theme::Primary::Main + ";"
        "}"
    );

    claimsCardLayout->addWidget(m_claimsTable);
    claimsTabLayout->addWidget(claimsCard);
    m_tabWidget->addTab(claimsTab, "🏷️ Claim Management");

    // ==========================================
    // TAB 1: ITEM INVENTORY
    // ==========================================
    QWidget *inventoryTab = new QWidget();
    QVBoxLayout *invTabLayout = new QVBoxLayout(inventoryTab);
    invTabLayout->setContentsMargins(0, 16, 0, 0);
    invTabLayout->setSpacing(16);

    // KPI Metrics Stat Cards Row
    QHBoxLayout *statsRow = new QHBoxLayout();
    statsRow->setSpacing(14);
    m_totalCard = new StatCardWidget("Total Registry Records", "📦", StatCardWidget::Indigo, inventoryTab);
    m_lostCard = new StatCardWidget("Active Lost Items", "🔴", StatCardWidget::Rose, inventoryTab);
    m_foundCard = new StatCardWidget("Recovered / Found", "🟢", StatCardWidget::Emerald, inventoryTab);
    m_activeCard = new StatCardWidget("Active Status Rate", "⚡", StatCardWidget::Amber, inventoryTab);

    statsRow->addWidget(m_totalCard);
    statsRow->addWidget(m_lostCard);
    statsRow->addWidget(m_foundCard);
    statsRow->addWidget(m_activeCard);
    invTabLayout->addLayout(statsRow);

    // Master Table Frame Card
    QFrame *tableCard = new QFrame(inventoryTab);
    tableCard->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 16px;"
        "   padding: 20px;"
        "}"
    );
    QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setSpacing(14);

    QHBoxLayout *tableHeaderRow = new QHBoxLayout();
    QLabel *tableTitle = new QLabel("Master Item Registry (Live Database)", tableCard);
    tableTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: " + Theme::Text::Primary + ";");

    m_recordCountLabel = new QLabel("0 records", tableCard);
    m_recordCountLabel->setStyleSheet("font-size: 13px; color: " + Theme::Text::Secondary + ";");

    m_searchEdit = new QLineEdit(tableCard);
    m_searchEdit->setPlaceholderText("🔍 Filter by ID, title, type, location, user...");
    m_searchEdit->setFixedWidth(280);
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 10px;"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   color: " + Theme::Text::Primary + ";"
        "}"
        "QLineEdit:focus { border: 2px solid " + Theme::Primary::Main + "; background-color: #FFFFFF; }"
    );
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AdminScreen::applyTableFilter);

    tableHeaderRow->addWidget(tableTitle);
    tableHeaderRow->addWidget(m_recordCountLabel);
    tableHeaderRow->addStretch();
    tableHeaderRow->addWidget(m_searchEdit);
    cardLayout->addLayout(tableHeaderRow);

    // Master Inventory Table: 8 Columns
    // ID | Item | Type | Location | Status | Reported By | Date | Action
    m_table = new QTableWidget(tableCard);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        "ID",
        "Item",
        "Type",
        "Location",
        "Status",
        "Reported By",
        "Date",
        "Action"
    });

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);

    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(true);
    m_table->setAlternatingRowColors(true);
    m_table->setMinimumHeight(380);

    m_table->setStyleSheet(
        "QTableWidget {"
        "   background-color: #FFFFFF;"
        "   alternate-background-color: #FAFCFE;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 10px;"
        "   font-size: 13px;"
        "   gridline-color: #F1F5F9;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: " + Theme::Text::Secondary + ";"
        "   font-weight: 700;"
        "   border: none;"
        "   border-bottom: 2px solid " + Theme::Border::Default + ";"
        "   padding: 10px 8px;"
        "   font-size: 11px;"
        "   text-transform: uppercase;"
        "   letter-spacing: 0.5px;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px 6px;"
        "   color: " + Theme::Text::Primary + ";"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: " + Theme::Primary::Subtle + ";"
        "   color: " + Theme::Primary::Main + ";"
        "}"
    );

    connect(m_table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        int row = item->row();
        if (row >= 0 && row < m_adminItems.size()) {
            emit viewItemDetails(m_adminItems[row]);
        }
    });

    cardLayout->addWidget(m_table);
    invTabLayout->addWidget(tableCard);
    m_tabWidget->addTab(inventoryTab, "📦 Item Inventory");

    contentLayout->addWidget(m_tabWidget);
    rootLayout->addWidget(m_contentBox);

    onClaimFilterBtnClicked("all");
    checkPermissions();
}

void AdminScreen::setLoading(bool loading)
{
    m_loadingBox->setVisible(loading);
    m_refreshBtn->setEnabled(!loading);
    if (loading) {
        m_errorBox->hide();
    }
}

void AdminScreen::showError(int statusCode, const QString &errorMessage)
{
    Q_UNUSED(statusCode);
    setLoading(false);
    m_errorLabel->setText(errorMessage.isEmpty() ? "Encountered an error fetching administrator records." : errorMessage);
    m_errorBox->show();
}

void AdminScreen::refreshData()
{
    if (!m_apiClient || !AuthManager::instance().isAdmin()) return;

    setLoading(true);

    // 1. Fetch Inventory Items
    m_apiClient->getAdminItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            populateTable(items);
        },
        [this](int statusCode, const QString &errorMessage) {
            showError(statusCode, errorMessage);
        }
    );

    // 2. Fetch Claims
    refreshClaims();
}

void AdminScreen::refreshClaims()
{
    if (!m_apiClient || !AuthManager::instance().isAdmin()) return;

    m_apiClient->getAdminClaims(
        "", // fetch all so we can client filter
        [this](const QVector<Claim> &claims, const ClaimStats &stats) {
            setLoading(false);
            m_adminClaims = claims;
            m_claimStats = stats;

            // Update KPI cards
            if (m_pendingClaimsCard) m_pendingClaimsCard->setValue(QString::number(stats.pending));
            if (m_approvedClaimsCard) m_approvedClaimsCard->setValue(QString::number(stats.approved));
            if (m_handedOverClaimsCard) m_handedOverClaimsCard->setValue(QString::number(stats.handedOver));
            if (m_rejectedClaimsCard) m_rejectedClaimsCard->setValue(QString::number(stats.rejected));

            // Update Header Pending Badge
            if (m_pendingBadgeLabel) {
                if (stats.pending > 0) {
                    m_pendingBadgeLabel->setText(QString("🔔 %1 Pending").arg(stats.pending));
                    m_pendingBadgeLabel->setStyleSheet("background-color: #FEF3C7; color: #B45309; font-size: 12px; font-weight: 700; padding: 4px 10px; border-radius: 12px; border: 1px solid #FCD34D;");
                } else {
                    m_pendingBadgeLabel->setText("✅ 0 Pending");
                    m_pendingBadgeLabel->setStyleSheet("background-color: #ECFDF5; color: #047857; font-size: 12px; font-weight: 700; padding: 4px 10px; border-radius: 12px; border: 1px solid #A7F3D0;");
                }
            }

            applyClaimsFilter();
        },
        [this](int statusCode, const QString &errorMessage) {
            showError(statusCode, errorMessage);
        }
    );
}

void AdminScreen::pollPendingClaims()
{
    // 1. Return immediately if the user is not an administrator or if screen is not visible
    if (!m_apiClient || !AuthManager::instance().isAdmin() || !isVisible()) {
        return;
    }

    // 2. Prevent overlapping polling requests
    if (m_isPolling) {
        return;
    }

    m_isPolling = true;

    // 3. Fetch current pending claims from existing endpoint GET /api/admin/claims?status=pending
    m_apiClient->getAdminClaims(
        "pending",
        [this](const QVector<Claim> &claims, const ClaimStats &stats) {
            // Always reset m_isPolling upon completion
            m_isPolling = false;

            // Extract each pending claim's unique ID
            QSet<int> currentPendingIds;
            for (const Claim &c : claims) {
                currentPendingIds.insert(c.id);
            }

            // Update Header Pending Badge
            if (m_pendingBadgeLabel) {
                if (stats.pending > 0) {
                    m_pendingBadgeLabel->setText(QString("🔔 %1 Pending").arg(stats.pending));
                    m_pendingBadgeLabel->setStyleSheet("background-color: #FEF3C7; color: #B45309; font-size: 12px; font-weight: 700; padding: 4px 10px; border-radius: 12px; border: 1px solid #FCD34D;");
                } else {
                    m_pendingBadgeLabel->setText("✅ 0 Pending");
                    m_pendingBadgeLabel->setStyleSheet("background-color: #ECFDF5; color: #047857; font-size: 12px; font-weight: 700; padding: 4px 10px; border-radius: 12px; border: 1px solid #A7F3D0;");
                }
            }

            // 4. On the FIRST successful poll:
            // Populate m_knownPendingClaimIds with existing pending claim IDs and suppress old alerts
            if (m_isFirstPoll) {
                m_knownPendingClaimIds = currentPendingIds;
                m_isFirstPoll = false;

                // Update KPI cards with initial pending claim statistics
                if (m_pendingClaimsCard) m_pendingClaimsCard->setValue(QString::number(stats.pending));
                if (m_approvedClaimsCard) m_approvedClaimsCard->setValue(QString::number(stats.approved));
                if (m_handedOverClaimsCard) m_handedOverClaimsCard->setValue(QString::number(stats.handedOver));
                if (m_rejectedClaimsCard) m_rejectedClaimsCard->setValue(QString::number(stats.rejected));
                return;
            }

            // 5. On subsequent successful polls:
            // Compare current pending claim IDs with m_knownPendingClaimIds to detect only new claims
            bool hasNewClaims = false;
            for (const Claim &c : claims) {
                if (!m_knownPendingClaimIds.contains(c.id)) {
                    hasNewClaims = true;

                    QString claimant = !c.claimantName.trimmed().isEmpty()
                                           ? c.claimantName.trimmed()
                                           : QString("User #%1").arg(c.userId);
                    QString item = !c.itemTitle.trimmed().isEmpty()
                                       ? QString(" for \"%1\"").arg(c.itemTitle.trimmed())
                                       : "";

                    QString msg = QString("🔔 New claim request received from %1%2 (%3)")
                                      .arg(claimant, item, c.claimReference);

                    emit showToast(msg, ToastType::Info);
                }
            }

            // 6. Maintain tracking set accuracy:
            m_knownPendingClaimIds = currentPendingIds;

            // 7. Refresh AdminScreen claim data/table if new claims were detected
            if (hasNewClaims) {
                refreshClaims();
            } else {
                // Keep KPI cards live
                if (m_pendingClaimsCard) m_pendingClaimsCard->setValue(QString::number(stats.pending));
                if (m_approvedClaimsCard) m_approvedClaimsCard->setValue(QString::number(stats.approved));
                if (m_handedOverClaimsCard) m_handedOverClaimsCard->setValue(QString::number(stats.handedOver));
                if (m_rejectedClaimsCard) m_rejectedClaimsCard->setValue(QString::number(stats.rejected));
            }
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            Q_UNUSED(errorMessage);
            // Always reset m_isPolling on failure as well
            m_isPolling = false;
        }
    );
}

void AdminScreen::onClaimFilterBtnClicked(const QString& filter)
{
    m_currentClaimsFilter = filter;

    auto updateBtn = [](QPushButton* btn, bool active, bool isPendingBtn = false) {
        if (!btn) return;
        if (isPendingBtn) {
            if (active) {
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #D97706; color: white; border: none;"
                    "   border-radius: 16px; font-size: 12px; font-weight: 700; padding: 6px 14px;"
                    "}"
                );
            } else {
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #FFFBEB; color: #B45309; border: 1px solid #FCD34D;"
                    "   border-radius: 16px; font-size: 12px; font-weight: 600; padding: 6px 14px;"
                    "}"
                    "QPushButton:hover { background-color: #FEF3C7; color: #92400E; border-color: #F59E0B; }"
                );
            }
        } else {
            if (active) {
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: " + Theme::Primary::Main + "; color: white; border: none;"
                    "   border-radius: 16px; font-size: 12px; font-weight: 600; padding: 6px 14px;"
                    "}"
                );
            } else {
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #FFFFFF; color: " + Theme::Text::Secondary + "; border: 1px solid " + Theme::Border::Default + ";"
                    "   border-radius: 16px; font-size: 12px; font-weight: 500; padding: 6px 14px;"
                    "}"
                    "QPushButton:hover { background-color: " + Theme::Primary::Subtle + "; color: " + Theme::Primary::Main + "; border-color: " + Theme::Primary::Border + "; }"
                );
            }
        }
    };

    updateBtn(m_cBtnAll, filter == "all", false);
    updateBtn(m_cBtnPending, filter == "pending", true);
    updateBtn(m_cBtnApproved, filter == "approved", false);
    updateBtn(m_cBtnHandedOver, filter == "handed_over", false);
    updateBtn(m_cBtnRejected, filter == "rejected", false);

    applyClaimsFilter();
}

void AdminScreen::applyClaimsFilter()
{
    QString query = m_claimsSearchEdit ? m_claimsSearchEdit->text().trimmed().toLower() : "";

    QVector<Claim> filtered;
    for (const Claim& c : m_adminClaims) {
        if (m_currentClaimsFilter != "all" && c.status.compare(m_currentClaimsFilter, Qt::CaseInsensitive) != 0) {
            continue;
        }

        if (!query.isEmpty()) {
            bool matches = c.claimReference.toLower().contains(query) ||
                           c.itemTitle.toLower().contains(query) ||
                           c.claimantName.toLower().contains(query) ||
                           c.claimantEmail.toLower().contains(query) ||
                           c.lastSeenLocation.toLower().contains(query);
            if (!matches) continue;
        }

        filtered.append(c);
    }

    populateClaimsTable(filtered);
}

void AdminScreen::populateClaimsTable(const QVector<Claim> &claims)
{
    if (!m_claimsTable) return;

    m_claimsTable->setRowCount(0);
    m_claimsTable->clearSpans();
    m_claimsCountLabel->setText(QString("%1 claim%2").arg(claims.size()).arg(claims.size() == 1 ? "" : "s"));

    // Empty state
    if (claims.isEmpty()) {
        m_claimsTable->setRowCount(1);
        m_claimsTable->setSpan(0, 0, 1, 7);
        QString emptyMsg;
        if (m_currentClaimsFilter == "pending") {
            emptyMsg = "🎉 No pending claims — All claim requests have been reviewed!";
        } else if (m_currentClaimsFilter == "approved") {
            emptyMsg = "📦 No items awaiting pickup.";
        } else if (m_currentClaimsFilter == "handed_over") {
            emptyMsg = "🤝 No claims have been handed over yet.";
        } else if (m_currentClaimsFilter == "rejected") {
            emptyMsg = "✅ No rejected claim requests.";
        } else {
            emptyMsg = "🔍 No claim requests match the current filter or search criteria.";
        }
        QTableWidgetItem *emptyItem = new QTableWidgetItem(emptyMsg);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QBrush(QColor("#64748B")));
        QFont font = emptyItem->font();
        font.setPointSize(11);
        font.setItalic(true);
        emptyItem->setFont(font);
        emptyItem->setFlags(Qt::NoItemFlags);
        m_claimsTable->setItem(0, 0, emptyItem);
        m_claimsTable->setRowHeight(0, 120);
        return;
    }

    for (int i = 0; i < claims.size(); ++i) {
        const Claim& c = claims[i];
        m_claimsTable->insertRow(i);

        // 0. Reference
        QTableWidgetItem *refItem = new QTableWidgetItem(c.claimReference);
        refItem->setFont(QFont("Monospace", 9, QFont::Bold));
        refItem->setTextAlignment(Qt::AlignCenter);
        m_claimsTable->setItem(i, 0, refItem);

        // 1. Target Item
        QString itemDesc = c.itemTitle.isEmpty() ? QString("Item #%1").arg(c.itemId) : c.itemTitle;
        if (!c.itemLocation.isEmpty()) {
            itemDesc += QString(" (%1)").arg(c.itemLocation);
        }
        QTableWidgetItem *itemCell = new QTableWidgetItem(itemDesc);
        itemCell->setToolTip(QString("Item #%1: %2").arg(c.itemId).arg(c.itemTitle));
        m_claimsTable->setItem(i, 1, itemCell);

        // 2. Claimant
        QString claimantStr = c.claimantName.isEmpty() ? QString("User #%1").arg(c.userId) : c.claimantName;
        if (!c.claimantEmail.isEmpty()) {
            claimantStr += QString(" (%1)").arg(c.claimantEmail);
        }
        m_claimsTable->setItem(i, 2, new QTableWidgetItem(claimantStr));

        // 3. Claim Date
        QTableWidgetItem *dateItem = new QTableWidgetItem(c.createdAt);
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_claimsTable->setItem(i, 3, dateItem);

        // 4. Status Badge Widget
        QWidget *badgeWidget = new QWidget();
        QHBoxLayout *badgeLayout = new QHBoxLayout(badgeWidget);
        badgeLayout->setContentsMargins(4, 4, 4, 4);
        badgeLayout->setAlignment(Qt::AlignCenter);

        QLabel *statusLabel = new QLabel(c.statusDisplayName(), badgeWidget);
        statusLabel->setStyleSheet(QString(
            "background-color: %1; color: %2; font-size: 11px; font-weight: bold; padding: 4px 10px; border-radius: 6px; border: 1px solid %2;"
        ).arg(c.statusBadgeBg(), c.statusColor()));
        badgeLayout->addWidget(statusLabel);
        m_claimsTable->setCellWidget(i, 4, badgeWidget);

        // 5. Pickup Station
        QString pickupLoc = c.pickupLocation.isEmpty() ? "Admin Block - Room 102" : c.pickupLocation;
        QTableWidgetItem *pickupItem = new QTableWidgetItem(pickupLoc);
        pickupItem->setTextAlignment(Qt::AlignCenter);
        m_claimsTable->setItem(i, 5, pickupItem);

        // 6. Actions Column: Real QPushButton Controls
        QWidget *actionWidget = new QWidget();
        actionWidget->setMinimumHeight(40);
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(6, 4, 6, 4);
        actionLayout->setSpacing(6);
        actionLayout->setAlignment(Qt::AlignCenter);

        if (c.isPending()) {
            QPushButton *reviewBtn = new QPushButton("Review", actionWidget);
            reviewBtn->setMinimumSize(90, 32);
            reviewBtn->setCursor(Qt::PointingHandCursor);
            reviewBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: " + Theme::Primary::Main + ";"
                "   color: white;"
                "   font-size: 12px;"
                "   font-weight: 600;"
                "   padding: 6px 16px;"
                "   border-radius: 6px;"
                "   border: none;"
                "}"
                "QPushButton:hover { background-color: " + Theme::Primary::Dark + "; }"
            );
            connect(reviewBtn, &QPushButton::clicked, this, [this, c]() {
                qDebug() << "REVIEW CLICKED"
                         << "Claim ID:" << c.id
                         << "Claim Reference:" << c.claimReference;

                AdminClaimReviewDialog *dlg = new AdminClaimReviewDialog(c, m_apiClient, this);
                connect(dlg, &AdminClaimReviewDialog::claimApproved, this, [this](const QString& ref) {
                    Q_UNUSED(ref);
                    emit showToast("Claim approved successfully.", ToastType::Success);
                    refreshClaims();
                });
                connect(dlg, &AdminClaimReviewDialog::claimRejected, this, [this](const QString& ref) {
                    Q_UNUSED(ref);
                    emit showToast("Claim rejected successfully.", ToastType::Success);
                    refreshClaims();
                });
                connect(dlg, &AdminClaimReviewDialog::claimHandedOver, this, [this](const QString& ref) {
                    Q_UNUSED(ref);
                    emit showToast("Item handed over successfully.", ToastType::Success);
                    refreshClaims();
                });
                connect(dlg, &AdminClaimReviewDialog::claimUpdated, this, &AdminScreen::refreshData);
                dlg->setAttribute(Qt::WA_DeleteOnClose);
                dlg->exec();
            });
            actionLayout->addWidget(reviewBtn);
        } else if (c.isApproved()) {
            QPushButton *handoverBtn = new QPushButton("Mark Handed Over", actionWidget);
            handoverBtn->setMinimumSize(140, 32);
            handoverBtn->setCursor(Qt::PointingHandCursor);
            handoverBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: #10B981;"
                "   color: white;"
                "   font-size: 12px;"
                "   font-weight: 600;"
                "   padding: 6px 14px;"
                "   border-radius: 6px;"
                "   border: none;"
                "}"
                "QPushButton:hover { background-color: #059669; }"
            );
            connect(handoverBtn, &QPushButton::clicked, this, [this, c]() {
                qDebug() << "HANDOVER CLICKED"
                         << "Claim ID:" << c.id
                         << "Claim Reference:" << c.claimReference;
                openHandoverDialog(c);
            });
            actionLayout->addWidget(handoverBtn);
        } else if (c.isHandedOver()) {
            QPushButton *doneBtn = new QPushButton("Completed", actionWidget);
            doneBtn->setMinimumSize(90, 32);
            doneBtn->setEnabled(false);
            doneBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: #F1F5F9;"
                "   color: #94A3B8;"
                "   font-size: 12px;"
                "   font-weight: 600;"
                "   padding: 6px 14px;"
                "   border-radius: 6px;"
                "   border: 1px solid #CBD5E1;"
                "}"
            );
            actionLayout->addWidget(doneBtn);
        } else { // Rejected
            QPushButton *reasonBtn = new QPushButton("View Reason", actionWidget);
            reasonBtn->setMinimumSize(100, 32);
            reasonBtn->setCursor(Qt::PointingHandCursor);
            reasonBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: #64748B;"
                "   color: white;"
                "   font-size: 12px;"
                "   font-weight: 600;"
                "   padding: 6px 14px;"
                "   border-radius: 6px;"
                "   border: none;"
                "}"
                "QPushButton:hover { background-color: #475569; }"
            );
            connect(reasonBtn, &QPushButton::clicked, this, [this, c]() {
                showRejectionReasonDialog(c);
            });
            actionLayout->addWidget(reasonBtn);
        }

        m_claimsTable->setRowHeight(i, 48);
        m_claimsTable->setCellWidget(i, 6, actionWidget);
    }
    m_claimsTable->setColumnWidth(6, 180);
}

void AdminScreen::openHandoverDialog(const Claim &claim)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Confirm Handover");
    dialog.setFixedWidth(440);
    dialog.setStyleSheet("background-color: #FFFFFF; font-family: 'Segoe UI', sans-serif;");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    QLabel *titleLabel = new QLabel("Confirm Handover", &dialog);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: " + Theme::Text::Primary + ";");
    layout->addWidget(titleLabel);

    QFrame *infoFrame = new QFrame(&dialog);
    infoFrame->setStyleSheet("background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 10px; padding: 14px;");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    infoLayout->setSpacing(8);

    auto addRow = [infoLayout](const QString &label, const QString &val) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #64748B; width: 80px;");
        QLabel *v = new QLabel(val);
        v->setStyleSheet("font-size: 13px; font-weight: 700; color: #1E293B;");
        v->setWordWrap(true);
        row->addWidget(lbl);
        row->addWidget(v, 1);
        infoLayout->addLayout(row);
    };

    addRow("Item:", claim.itemTitle.isEmpty() ? QString("Item #%1").arg(claim.itemId) : claim.itemTitle);
    addRow("Claimant:", claim.claimantName.isEmpty() ? QString("User #%1").arg(claim.userId) : claim.claimantName);
    layout->addWidget(infoFrame);

    QLabel *confirmMsg = new QLabel("Confirm that the item has physically been handed over to the claimant.", &dialog);
    confirmMsg->setWordWrap(true);
    confirmMsg->setStyleSheet("font-size: 13px; color: " + Theme::Text::Primary + "; line-height: 140%;");
    layout->addWidget(confirmMsg);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Cancel", &dialog);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #F1F5F9; color: #475569; font-size: 13px; font-weight: 600; padding: 10px 18px; border-radius: 8px; border: none; }"
        "QPushButton:hover { background-color: #E2E8F0; }"
    );
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    QPushButton *confirmBtn = new QPushButton("Confirm Handover", &dialog);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(
        "QPushButton { background-color: #10B981; color: white; font-size: 13px; font-weight: 700; padding: 10px 18px; border-radius: 8px; border: none; }"
        "QPushButton:hover { background-color: #059669; }"
    );
    connect(confirmBtn, &QPushButton::clicked, [&]() {
        confirmBtn->setEnabled(false);
        confirmBtn->setText("Processing...");

        qDebug() << "HANDOVER CLICKED"
                 << "Claim ID:"
                 << claim.id
                 << "Claim Reference:"
                 << claim.claimReference;

        m_apiClient->handoverClaim(
            claim.id,
            [this, &dialog](const QJsonObject &json) {
                Q_UNUSED(json);
                dialog.accept();
                emit showToast("Item handed over successfully.", ToastType::Success);
                refreshClaims();
                refreshData();
            },
            [this, confirmBtn, &dialog](int statusCode, const QString &errorMessage) {
                Q_UNUSED(statusCode);
                confirmBtn->setEnabled(true);
                confirmBtn->setText("Confirm Handover");
                QMessageBox::critical(&dialog, "Handover Failed", errorMessage.isEmpty() ? "Failed to record handover." : errorMessage);
            }
        );
    });

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(confirmBtn);
    layout->addLayout(btnLayout);

    dialog.exec();
}

void AdminScreen::showRejectionReasonDialog(const Claim &claim)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Claim Rejection Details");
    dialog.setFixedWidth(440);
    dialog.setStyleSheet("background-color: #FFFFFF;");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    QLabel *titleLabel = new QLabel("❌ Claim Rejection Details", &dialog);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: " + Theme::Text::Primary + ";");
    layout->addWidget(titleLabel);

    QFrame *infoFrame = new QFrame(&dialog);
    infoFrame->setStyleSheet("background-color: #FFF1F2; border: 1px solid #FECDD3; border-radius: 10px; padding: 14px;");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    infoLayout->setSpacing(8);

    auto addRow = [infoLayout](const QString &label, const QString &val) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 12px; font-weight: 600; color: #9F1239; width: 90px;");
        QLabel *v = new QLabel(val);
        v->setStyleSheet("font-size: 12px; font-weight: 700; color: #881337;");
        v->setWordWrap(true);
        row->addWidget(lbl);
        row->addWidget(v, 1);
        infoLayout->addLayout(row);
    };

    addRow("Claim Ref:", claim.claimReference);
    addRow("Item:", claim.itemTitle.isEmpty() ? QString("Item #%1").arg(claim.itemId) : claim.itemTitle);
    addRow("Claimant:", claim.claimantName.isEmpty() ? QString("User #%1").arg(claim.userId) : claim.claimantName);
    addRow("Reviewed At:", claim.reviewedAt.isEmpty() ? "N/A" : claim.reviewedAt);

    QString notes = claim.rejectionReason.trimmed().isEmpty() ? "No specific reason provided by reviewer." : claim.rejectionReason.trimmed();
    QLabel *notesTitle = new QLabel("Rejection Reason:", infoFrame);
    notesTitle->setStyleSheet("font-size: 12px; font-weight: 700; color: #9F1239; margin-top: 6px;");
    QLabel *notesVal = new QLabel(notes, infoFrame);
    notesVal->setStyleSheet("font-size: 13px; color: #881337; background-color: #FFFFFF; border: 1px solid #FDA4AF; border-radius: 6px; padding: 8px;");
    notesVal->setWordWrap(true);

    infoLayout->addWidget(notesTitle);
    infoLayout->addWidget(notesVal);
    layout->addWidget(infoFrame);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *closeBtn = new QPushButton("Close", &dialog);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #64748B; color: white; font-size: 13px; font-weight: 600; padding: 8px 20px; border-radius: 8px; border: none; }"
        "QPushButton:hover { background-color: #475569; }"
    );
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    dialog.exec();
}

void AdminScreen::populateTable(const QVector<Item> &items)
{
    m_adminItems = items;
    setLoading(false);

    int totalCount = items.size();
    int lostCount = 0;
    int foundCount = 0;
    int activeCount = 0;

    for (const Item &item : items) {
        if (item.isLost()) {
            lostCount++;
        } else if (item.isFound()) {
            foundCount++;
        }

        if (item.status.toLower() == "active") {
            activeCount++;
        }
    }

    m_totalCard->setValue(QString::number(totalCount));
    m_lostCard->setValue(QString::number(lostCount));
    m_foundCard->setValue(QString::number(foundCount));
    int pct = totalCount > 0 ? static_cast<int>((static_cast<double>(activeCount) / totalCount) * 100.0) : 0;
    m_activeCard->setValue(QString("%1%").arg(pct));

    applyTableFilter();
}

void AdminScreen::applyTableFilter()
{
    QString query = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : "";

    QVector<Item> filtered;
    for (const Item &item : m_adminItems) {
        if (query.isEmpty()) {
            filtered.append(item);
            continue;
        }

        bool match = QString::number(item.id).contains(query) ||
                     item.title.toLower().contains(query) ||
                     item.description.toLower().contains(query) ||
                     item.type.toLower().contains(query) ||
                     item.location.toLower().contains(query) ||
                     item.status.toLower().contains(query) ||
                     QString::number(item.userId).contains(query);

        if (match) {
            filtered.append(item);
        }
    }

    m_table->setRowCount(0);
    m_table->clearSpans();
    m_recordCountLabel->setText(QString("%1 records").arg(filtered.size()));

    if (filtered.isEmpty()) {
        m_table->setRowCount(1);
        m_table->setSpan(0, 0, 1, 8);
        QTableWidgetItem *emptyItem = new QTableWidgetItem("📦 No items found matching the current search criteria.");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QBrush(QColor("#64748B")));
        QFont font = emptyItem->font();
        font.setPointSize(11);
        font.setItalic(true);
        emptyItem->setFont(font);
        emptyItem->setFlags(Qt::NoItemFlags);
        m_table->setItem(0, 0, emptyItem);
        m_table->setRowHeight(0, 120);
        return;
    }

    for (int row = 0; row < filtered.size(); ++row) {
        const Item &item = filtered[row];
        m_table->insertRow(row);

        // 0. ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(item.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, idItem);

        // 1. Item
        m_table->setItem(row, 1, new QTableWidgetItem(item.title));

        // 2. Type
        QTableWidgetItem *typeItem = new QTableWidgetItem(item.isLost() ? "🔴 Lost" : "🟢 Found");
        typeItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, typeItem);

        // 3. Location
        m_table->setItem(row, 3, new QTableWidgetItem(item.location));

        // 4. Status
        QTableWidgetItem *statusItem = new QTableWidgetItem(item.status.toUpper());
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 4, statusItem);

        // 5. Reported By
        QTableWidgetItem *userItem = new QTableWidgetItem(QString("User #%1").arg(item.userId));
        userItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 5, userItem);

        // 6. Date
        QTableWidgetItem *dateItem = new QTableWidgetItem(item.createdAt);
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, dateItem);

        // 7. Action: [View Details]
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setAlignment(Qt::AlignCenter);

        QPushButton *viewBtn = new QPushButton("🔍 View Details", actionWidget);
        viewBtn->setCursor(Qt::PointingHandCursor);
        viewBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Theme::Primary::Main + ";"
            "   color: white;"
            "   font-size: 11px;"
            "   font-weight: 600;"
            "   padding: 5px 12px;"
            "   border-radius: 6px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: " + Theme::Primary::Dark + "; }"
        );
        connect(viewBtn, &QPushButton::clicked, this, [this, item]() {
            emit viewItemDetails(item);
        });
        actionLayout->addWidget(viewBtn);
        m_table->setCellWidget(row, 7, actionWidget);
    }
}
