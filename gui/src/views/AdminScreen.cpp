#include "AdminScreen.h"
#include "../auth/AuthManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QFrame>
#include <QShowEvent>

AdminScreen::AdminScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_totalCard(nullptr)
    , m_lostCard(nullptr)
    , m_foundCard(nullptr)
    , m_activeCard(nullptr)
    , m_searchEdit(nullptr)
    , m_refreshBtn(nullptr)
    , m_recordCountLabel(nullptr)
    , m_forbiddenBox(nullptr)
    , m_contentBox(nullptr)
    , m_loadingBox(nullptr)
    , m_progressBar(nullptr)
    , m_errorBox(nullptr)
    , m_errorLabel(nullptr)
    , m_retryBtn(nullptr)
    , m_table(nullptr)
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

    QLabel *pageTitle = new QLabel("🛡️ Master Inventory Administration", this);
    pageTitle->setStyleSheet("font-size: 24px; font-weight: 800; color: #0F172A;");

    QLabel *pageSub = new QLabel("Restricted administrative console for campus property tracking and audit.", this);
    pageSub->setStyleSheet("font-size: 13px; color: #64748B;");

    titleLayout->addWidget(pageTitle);
    titleLayout->addWidget(pageSub);
    headerRow->addLayout(titleLayout);
    headerRow->addStretch();

    m_refreshBtn = new QPushButton("🔄 Refresh Registry", this);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: white;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, &AdminScreen::refreshData);
    headerRow->addWidget(m_refreshBtn);

    rootLayout->addLayout(headerRow);

    // 1. FORBIDDEN ACCESS BARRIER CARD (Shown if non-admin or unauthenticated)
    m_forbiddenBox = new QFrame(this);
    m_forbiddenBox->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 48px;"
        "}"
    );
    QVBoxLayout *forbiddenLayout = new QVBoxLayout(m_forbiddenBox);
    forbiddenLayout->setAlignment(Qt::AlignCenter);
    forbiddenLayout->setSpacing(14);

    QLabel *lockIcon = new QLabel("🔒", m_forbiddenBox);
    lockIcon->setStyleSheet("font-size: 56px; background: transparent;");
    lockIcon->setAlignment(Qt::AlignCenter);

    QLabel *forbidTitle = new QLabel("Administrator Access Required", m_forbiddenBox);
    forbidTitle->setStyleSheet("font-size: 22px; font-weight: 800; color: #DC2626; background: transparent;");
    forbidTitle->setAlignment(Qt::AlignCenter);

    QLabel *forbidSub = new QLabel(
        "This section is restricted to campus administrators.\n"
        "You must be signed in with an authorized account having the 'admin' role to access the master registry.",
        m_forbiddenBox
    );
    forbidSub->setStyleSheet("font-size: 14px; color: #64748B; background: transparent;");
    forbidSub->setAlignment(Qt::AlignCenter);

    QPushButton *loginAdminBtn = new QPushButton("🔑 Sign In with Admin Account", m_forbiddenBox);
    loginAdminBtn->setCursor(Qt::PointingHandCursor);
    loginAdminBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: white;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 22px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(loginAdminBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(3); });

    forbiddenLayout->addWidget(lockIcon);
    forbiddenLayout->addWidget(forbidTitle);
    forbiddenLayout->addWidget(forbidSub);
    forbiddenLayout->addSpacing(8);
    forbiddenLayout->addWidget(loginAdminBtn);

    rootLayout->addWidget(m_forbiddenBox);

    // 2. MAIN AUTHORIZED ADMIN CONTENT CONTAINER
    m_contentBox = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentBox);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(20);

    // KPI Metrics Stat Cards Row
    QHBoxLayout *statsRow = new QHBoxLayout();
    statsRow->setSpacing(14);
    m_totalCard = new StatCardWidget("Total Registry Records", "📦", StatCardWidget::Indigo, m_contentBox);
    m_lostCard = new StatCardWidget("Active Lost Items", "🔴", StatCardWidget::Rose, m_contentBox);
    m_foundCard = new StatCardWidget("Recovered / Found", "🟢", StatCardWidget::Emerald, m_contentBox);
    m_activeCard = new StatCardWidget("Active Status Rate", "⚡", StatCardWidget::Amber, m_contentBox);

    statsRow->addWidget(m_totalCard);
    statsRow->addWidget(m_lostCard);
    statsRow->addWidget(m_foundCard);
    statsRow->addWidget(m_activeCard);
    contentLayout->addLayout(statsRow);

    // Loading State Box
    m_loadingBox = new QWidget(m_contentBox);
    QVBoxLayout *loadLayout = new QVBoxLayout(m_loadingBox);
    loadLayout->setContentsMargins(0, 8, 0, 8);
    loadLayout->setSpacing(8);

    QLabel *loadLabel = new QLabel("⏳ Querying master registry from backend server...", m_loadingBox);
    loadLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4F46E5;");
    loadLabel->setAlignment(Qt::AlignCenter);

    m_progressBar = new QProgressBar(m_loadingBox);
    m_progressBar->setRange(0, 0);
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #E2E8F0; border-radius: 2px; border: none; }"
        "QProgressBar::chunk { background-color: #4F46E5; border-radius: 2px; }"
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
        "   border-radius: 8px;"
        "   padding: 14px;"
        "}"
    );
    QHBoxLayout *errorLayout = new QHBoxLayout(m_errorBox);
    m_errorLabel = new QLabel(m_errorBox);
    m_errorLabel->setStyleSheet("color: #DC2626; font-size: 13px; font-weight: 600; background: transparent;");

    m_retryBtn = new QPushButton("Retry Request", m_errorBox);
    m_retryBtn->setCursor(Qt::PointingHandCursor);
    m_retryBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #DC2626;"
        "   color: white;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "   padding: 6px 14px;"
        "   border-radius: 6px;"
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

    // Master Table Frame Card
    QFrame *tableCard = new QFrame(m_contentBox);
    tableCard->setStyleSheet(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 20px;"
        "}"
    );
    QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setSpacing(14);

    QHBoxLayout *tableHeaderRow = new QHBoxLayout();
    QLabel *tableTitle = new QLabel("Master Item Registry (Live Database)", tableCard);
    tableTitle->setStyleSheet("font-size: 16px; font-weight: 800; color: #0F172A;");

    m_recordCountLabel = new QLabel("0 records", tableCard);
    m_recordCountLabel->setStyleSheet("font-size: 13px; color: #64748B;");

    m_searchEdit = new QLineEdit(tableCard);
    m_searchEdit->setPlaceholderText("🔍 Filter by ID, title, type, location, user...");
    m_searchEdit->setFixedWidth(280);
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus { border: 1.5px solid #4F46E5; background-color: #FFFFFF; }"
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

    m_table->setStyleSheet(
        "QTableWidget {"
        "   background-color: #FFFFFF;"
        "   alternate-background-color: #F8FAFC;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   font-size: 13px;"
        "   gridline-color: #F1F5F9;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   font-weight: 700;"
        "   border: none;"
        "   border-bottom: 2px solid #E2E8F0;"
        "   padding: 10px 8px;"
        "   font-size: 11px;"
        "   text-transform: uppercase;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px 6px;"
        "   color: #1E293B;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #EEF2FF;"
        "   color: #4F46E5;"
        "}"
    );

    connect(m_table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        int row = item->row();
        if (row >= 0 && row < m_adminItems.size()) {
            emit viewItemDetails(m_adminItems[row]);
        }
    });

    cardLayout->addWidget(m_table);
    contentLayout->addWidget(tableCard);

    rootLayout->addWidget(m_contentBox);

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
    m_loadingBox->hide();
    if (statusCode == 401) {
        m_errorLabel->setText("🔒 Unauthorized (401): Session expired. Please sign in with an admin account.");
    } else if (statusCode == 403) {
        m_errorLabel->setText("🚫 Forbidden (403): You do not have administrator permissions to access this endpoint.");
    } else {
        m_errorLabel->setText(QString("❌ Error (%1): %2").arg(statusCode).arg(errorMessage));
    }
    m_errorBox->show();
}

void AdminScreen::refreshData()
{
    if (!AuthManager::instance().isAdmin()) {
        checkPermissions();
        return;
    }

    setLoading(true);
    m_errorBox->hide();

    // Sends GET /api/admin/items with Authorization: Bearer ADMIN_TOKEN
    m_apiClient->getAdminItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            setLoading(false);
            populateTable(items);
        },
        [this](int statusCode, const QString &errorMessage) {
            setLoading(false);
            showError(statusCode, errorMessage);
        }
    );
}

void AdminScreen::populateTable(const QVector<Item> &items)
{
    m_adminItems = items;

    int total = items.size();
    int lost = 0;
    int found = 0;
    int active = 0;

    for (const Item &it : items) {
        if (it.isLost()) lost++;
        else if (it.isFound()) found++;
        if (it.status.toLower() == "active" || it.status.isEmpty()) active++;
    }

    m_totalCard->setValue(total);
    m_lostCard->setValue(lost);
    m_foundCard->setValue(found);
    m_activeCard->setValue(active);

    m_recordCountLabel->setText(QString("%1 records in registry").arg(total));

    applyTableFilter();
}

void AdminScreen::applyTableFilter()
{
    QString filter = m_searchEdit->text().trimmed().toLower();

    m_table->setRowCount(0);

    for (const Item &it : m_adminItems) {
        if (!filter.isEmpty()) {
            bool match = QString::number(it.id).contains(filter) ||
                         it.title.toLower().contains(filter) ||
                         it.description.toLower().contains(filter) ||
                         it.location.toLower().contains(filter) ||
                         it.type.toLower().contains(filter) ||
                         it.status.toLower().contains(filter) ||
                         QString::number(it.userId).contains(filter);
            if (!match) continue;
        }

        int row = m_table->rowCount();
        m_table->insertRow(row);

        // 0. ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString("#%1").arg(it.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, idItem);

        // 1. Title
        QTableWidgetItem *titleItem = new QTableWidgetItem(it.title);
        titleItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        m_table->setItem(row, 1, titleItem);

        // 2. Description
        QTableWidgetItem *descItem = new QTableWidgetItem(it.description);
        m_table->setItem(row, 2, descItem);

        // 3. Type Badge
        QTableWidgetItem *typeItem = new QTableWidgetItem(it.isLost() ? "🔴 LOST" : "🟢 FOUND");
        typeItem->setTextAlignment(Qt::AlignCenter);
        if (it.isLost()) {
            typeItem->setForeground(QBrush(QColor("#DC2626")));
        } else {
            typeItem->setForeground(QBrush(QColor("#059669")));
        }
        m_table->setItem(row, 3, typeItem);

        // 4. Location
        QTableWidgetItem *locItem = new QTableWidgetItem(it.location);
        m_table->setItem(row, 4, locItem);

        // 5. Status
        QTableWidgetItem *statusItem = new QTableWidgetItem(it.status.isEmpty() ? "active" : it.status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 5, statusItem);

        // 6. User ID
        QTableWidgetItem *userItem = new QTableWidgetItem(QString("User %1").arg(it.userId));
        userItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 6, userItem);

        // 7. Created At
        QTableWidgetItem *dateItem = new QTableWidgetItem(it.createdAt.isEmpty() ? "N/A" : it.createdAt);
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 7, dateItem);
    }
}
