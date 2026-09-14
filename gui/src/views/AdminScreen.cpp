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

AdminScreen::AdminScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_currentClaimsFilter("all")
    , m_tabWidget(nullptr)
    , m_totalCard(nullptr)
    , m_lostCard(nullptr)
    , m_foundCard(nullptr)
    , m_activeCard(nullptr)
    , m_pendingClaimsCard(nullptr)
    , m_approvedClaimsCard(nullptr)
    , m_handedOverClaimsCard(nullptr)
    , m_totalClaimsCard(nullptr)
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
{
    setupUI();

    connect(&AuthManager::instance(), &AuthManager::authStateChanged, this, [this](bool loggedIn) {
        Q_UNUSED(loggedIn);
        checkPermissions();
    });
}

void AdminScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    checkPermissions();
}

void AdminScreen::checkPermissions()
{
    bool isAdmin = AuthManager::instance().isAdmin();
    m_forbiddenBox->setVisible(!isAdmin);
    m_contentBox->setVisible(isAdmin);

    if (isAdmin) {
        refreshData();
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
    titleLayout->setSpacing(2);

    QLabel *pageTitle = new QLabel("🛡️ Master Campus Administration & Claims Console", this);
    pageTitle->setStyleSheet("font-size: 24px; font-weight: 700; color: " + Theme::Text::Primary + "; background: transparent;");

    QLabel *pageSub = new QLabel("Restricted administrative console for campus property verification, claims approval, and master inventory audit.", this);
    pageSub->setStyleSheet("font-size: 13px; color: " + Theme::Text::Secondary + "; background: transparent;");

    titleLayout->addWidget(pageTitle);
    titleLayout->addWidget(pageSub);
    headerRow->addLayout(titleLayout);
    headerRow->addStretch();

    m_refreshBtn = new QPushButton("🔄 Refresh All", this);
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

    // ==========================================
    // TAB 1: CLAIM REQUESTS & VERIFICATION
    // ==========================================
    QWidget *claimsTab = new QWidget();
    QVBoxLayout *claimsTabLayout = new QVBoxLayout(claimsTab);
    claimsTabLayout->setContentsMargins(0, 16, 0, 0);
    claimsTabLayout->setSpacing(16);

    // Claims KPI Row
    QHBoxLayout *claimsStatsRow = new QHBoxLayout();
    claimsStatsRow->setSpacing(14);
    m_pendingClaimsCard = new StatCardWidget("Pending Review", "⏳", StatCardWidget::Amber, claimsTab);
    m_approvedClaimsCard = new StatCardWidget("Ready for Pickup", "🎫", StatCardWidget::Emerald, claimsTab);
    m_handedOverClaimsCard = new StatCardWidget("Handed Over (Completed)", "🤝", StatCardWidget::Indigo, claimsTab);
    m_totalClaimsCard = new StatCardWidget("Total Claim Files", "📋", StatCardWidget::Indigo, claimsTab);

    claimsStatsRow->addWidget(m_pendingClaimsCard);
    claimsStatsRow->addWidget(m_approvedClaimsCard);
    claimsStatsRow->addWidget(m_handedOverClaimsCard);
    claimsStatsRow->addWidget(m_totalClaimsCard);
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

    // Claims Filter & Search Bar
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
    m_cBtnPending = makeClaimFilterBtn("Pending Review", "pending");
    m_cBtnApproved = makeClaimFilterBtn("Ready for Pickup", "approved");
    m_cBtnHandedOver = makeClaimFilterBtn("Handed Over", "handed_over");
    m_cBtnRejected = makeClaimFilterBtn("Rejected", "rejected");

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
    m_claimsSearchEdit->setPlaceholderText("🔍 Search ref, claimant, item, email...");
    m_claimsSearchEdit->setFixedWidth(260);
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

    // Claims Table Widget
    m_claimsTable = new QTableWidget(claimsCard);
    m_claimsTable->setColumnCount(7);
    m_claimsTable->setHorizontalHeaderLabels({
        "Reference",
        "Target Item",
        "Claimant",
        "Claim Date",
        "Status",
        "Pickup Station",
        "Actions"
    });

    m_claimsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_claimsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

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
    m_tabWidget->addTab(claimsTab, "🏷️ Claim Requests & Handover");

    // ==========================================
    // TAB 2: MASTER INVENTORY
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

    // Master Table Widget: 8 Columns
    m_table = new QTableWidget(tableCard);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        "ID",
        "Title",
        "Description",
        "Type",
        "Location",
        "Status",
        "User ID",
        "Created At"
    });

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
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
    m_tabWidget->addTab(inventoryTab, "📦 Master Inventory");

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
            m_pendingClaimsCard->setValue(QString::number(stats.pending));
            m_approvedClaimsCard->setValue(QString::number(stats.approved));
            m_handedOverClaimsCard->setValue(QString::number(stats.handedOver));
            m_totalClaimsCard->setValue(QString::number(stats.total));

            applyClaimsFilter();
        },
        [this](int statusCode, const QString &errorMessage) {
            showError(statusCode, errorMessage);
        }
    );
}

void AdminScreen::onClaimFilterBtnClicked(const QString& filter)
{
    m_currentClaimsFilter = filter;

    auto updateBtn = [](QPushButton* btn, bool active) {
        if (!btn) return;
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
    };

    updateBtn(m_cBtnAll, filter == "all");
    updateBtn(m_cBtnPending, filter == "pending");
    updateBtn(m_cBtnApproved, filter == "approved");
    updateBtn(m_cBtnHandedOver, filter == "handed_over");
    updateBtn(m_cBtnRejected, filter == "rejected");

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
    m_claimsCountLabel->setText(QString("%1 claim%2").arg(claims.size()).arg(claims.size() == 1 ? "" : "s"));

    for (int i = 0; i < claims.size(); ++i) {
        const Claim& c = claims[i];
        m_claimsTable->insertRow(i);

        // 0. Reference
        QTableWidgetItem *refItem = new QTableWidgetItem(c.claimReference);
        refItem->setFont(QFont("Monospace", 9, QFont::Bold));
        m_claimsTable->setItem(i, 0, refItem);

        // 1. Target Item
        QString itemDesc = QString("#%1 %2 (%3)").arg(c.itemId).arg(c.itemTitle, c.itemLocation);
        m_claimsTable->setItem(i, 1, new QTableWidgetItem(itemDesc));

        // 2. Claimant
        QString claimantStr = QString("%1 (%2)").arg(c.claimantName.isEmpty() ? QString("User #%1").arg(c.userId) : c.claimantName, c.claimantEmail);
        m_claimsTable->setItem(i, 2, new QTableWidgetItem(claimantStr));

        // 3. Claim Date
        m_claimsTable->setItem(i, 3, new QTableWidgetItem(c.createdAt));

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

        // 5. Pickup Location
        m_claimsTable->setItem(i, 5, new QTableWidgetItem(c.pickupLocation));

        // 6. Action Button Widget: Review Claim
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setAlignment(Qt::AlignCenter);

        QPushButton *reviewBtn = new QPushButton(c.isPending() ? "⚡ Review & Verify" : "🔍 View Details", actionWidget);
        reviewBtn->setCursor(Qt::PointingHandCursor);
        reviewBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   font-size: 11px;"
            "   font-weight: 600;"
            "   padding: 6px 14px;"
            "   border-radius: 8px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: %2; }"
        ).arg(c.isPending() ? Theme::Primary::Main : "#64748B", c.isPending() ? Theme::Primary::Dark : "#475569"));

        connect(reviewBtn, &QPushButton::clicked, this, [this, c]() {
            AdminClaimReviewDialog dlg(c, m_apiClient, this);
            connect(&dlg, &AdminClaimReviewDialog::claimUpdated, this, &AdminScreen::refreshData);
            dlg.exec();
        });

        actionLayout->addWidget(reviewBtn);
        m_claimsTable->setCellWidget(i, 6, actionWidget);
    }
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
    m_recordCountLabel->setText(QString("%1 records").arg(filtered.size()));

    for (int row = 0; row < filtered.size(); ++row) {
        const Item &item = filtered[row];
        m_table->insertRow(row);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(item.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, idItem);

        m_table->setItem(row, 1, new QTableWidgetItem(item.title));
        m_table->setItem(row, 2, new QTableWidgetItem(item.description));

        QTableWidgetItem *typeItem = new QTableWidgetItem(item.isLost() ? "🔴 Lost" : "🟢 Found");
        typeItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 3, typeItem);

        m_table->setItem(row, 4, new QTableWidgetItem(item.location));

        QTableWidgetItem *statusItem = new QTableWidgetItem(item.status.toUpper());
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 5, statusItem);

        QTableWidgetItem *userItem = new QTableWidgetItem(QString("User #%1").arg(item.userId));
        userItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, userItem);

        QTableWidgetItem *dateItem = new QTableWidgetItem(item.createdAt);
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 7, dateItem);
    }
}
