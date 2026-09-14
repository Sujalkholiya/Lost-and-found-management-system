#include "BrowseScreen.h"
#include "../components/ItemCardWidget.h"
#include "../components/EmptyStateWidget.h"
#include "../models/CampusLocations.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>

BrowseScreen::BrowseScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_selectedTypeFilter("all")
    , m_searchEdit(nullptr)
    , m_allBtn(nullptr)
    , m_lostBtn(nullptr)
    , m_foundBtn(nullptr)
    , m_locationCombo(nullptr)
    , m_resetBtn(nullptr)
    , m_countLabel(nullptr)
    , m_loadingBox(nullptr)
    , m_loadingLabel(nullptr)
    , m_progressBar(nullptr)
    , m_errorBox(nullptr)
    , m_errorLabel(nullptr)
    , m_retryBtn(nullptr)
    , m_gridContainer(nullptr)
    , m_cardsGrid(nullptr)
{
    setupUI();
}

void BrowseScreen::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(36, 28, 36, 28);
    rootLayout->setSpacing(16);

    // 1. Page Header with Title & Action
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    QLabel *pageTitle = new QLabel("Browse Campus Items", this);
    pageTitle->setStyleSheet("font-size: 24px; font-weight: 800; color: #17345F; background: transparent;");

    QLabel *pageSubtitle = new QLabel("Search and filter all reported lost and found belongings across the university.", this);
    pageSubtitle->setStyleSheet("font-size: 13px; color: #6B7FA3; font-weight: 500; background: transparent;");

    titleLayout->addWidget(pageTitle);
    titleLayout->addWidget(pageSubtitle);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    QPushButton *reportNewBtn = new QPushButton("➕ Report an Item", this);
    reportNewBtn->setCursor(Qt::PointingHandCursor);
    reportNewBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );
    connect(reportNewBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); }); // 2 = Report
    headerLayout->addWidget(reportNewBtn);

    rootLayout->addLayout(headerLayout);

    // 2. Integrated Filter & Search Card
    QFrame *filterCard = new QFrame(this);
    filterCard->setStyleSheet(
        "QFrame#filterCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 14px;"
        "   padding: 4px;"
        "}"
    );
    filterCard->setObjectName("filterCard");

    QHBoxLayout *filterLayout = new QHBoxLayout(filterCard);
    filterLayout->setContentsMargins(12, 10, 12, 10);
    filterLayout->setSpacing(10);

    // Search Input Field
    m_searchEdit = new QLineEdit(filterCard);
    m_searchEdit->setPlaceholderText("🔍 Search title, description, or location...");
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 8px;"
        "   padding: 8px 14px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   color: #172B4D;"
        "   selection-background-color: #2563EB;"
        "   selection-color: #FFFFFF;"
        "}"
        "QLineEdit:hover { border-color: #CBD5E1; }"
        "QLineEdit:focus { border: 2px solid #2563EB; background-color: #FFFFFF; color: #172B4D; }"
    );
    m_searchEdit->setMinimumWidth(260);

    // Filter Buttons: All, Lost, Found
    m_allBtn = new QPushButton("All Items", filterCard);
    m_lostBtn = new QPushButton("🔴 Lost", filterCard);
    m_foundBtn = new QPushButton("🟢 Found", filterCard);

    m_allBtn->setCursor(Qt::PointingHandCursor);
    m_lostBtn->setCursor(Qt::PointingHandCursor);
    m_foundBtn->setCursor(Qt::PointingHandCursor);

    connect(m_allBtn, &QPushButton::clicked, this, [this]() { onFilterButtonClicked("all"); });
    connect(m_lostBtn, &QPushButton::clicked, this, [this]() { onFilterButtonClicked("lost"); });
    connect(m_foundBtn, &QPushButton::clicked, this, [this]() { onFilterButtonClicked("found"); });

    // Campus Location Dropdown (Centralized List)
    m_locationCombo = new QComboBox(filterCard);
    m_locationCombo->addItem("📍 All Campus Locations", "");
    for (const QString &loc : CampusLocations::predefinedLocations()) {
        m_locationCombo->addItem(loc, loc);
    }
    m_locationCombo->setStyleSheet(
        "QComboBox {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-size: 12px;"
        "   font-weight: 600;"
        "   color: #172B4D;"
        "}"
        "QComboBox:hover { border-color: #CBD5E1; }"
        "QComboBox:focus { border: 2px solid #2563EB; }"
        "QComboBox QAbstractItemView {"
        "   background-color: #FFFFFF;"
        "   color: #172B4D;"
        "   border: 1px solid #E2EAF4;"
        "   selection-background-color: #2563EB;"
        "   selection-color: #FFFFFF;"
        "   outline: none;"
        "   padding: 4px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   min-height: 28px;"
        "   padding: 4px 8px;"
        "   color: #172B4D;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: #EAF2FF;"
        "   color: #2563EB;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "}"
    );

    // Reset Filters Button
    m_resetBtn = new QPushButton("🔄 Reset", filterCard);
    m_resetBtn->setCursor(Qt::PointingHandCursor);
    m_resetBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F4F8FF;"
        "   color: #2563EB;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #EAF2FF; border-color: #CBD5E1; }"
    );
    connect(m_resetBtn, &QPushButton::clicked, this, &BrowseScreen::resetFilters);

    filterLayout->addWidget(m_searchEdit, 2);
    filterLayout->addWidget(m_allBtn);
    filterLayout->addWidget(m_lostBtn);
    filterLayout->addWidget(m_foundBtn);
    filterLayout->addWidget(m_locationCombo, 1);
    filterLayout->addWidget(m_resetBtn);

    rootLayout->addWidget(filterCard);

    // 3. Loading Indicator Box
    m_loadingBox = new QWidget(this);
    QVBoxLayout *loadingLayout = new QVBoxLayout(m_loadingBox);
    loadingLayout->setAlignment(Qt::AlignCenter);
    loadingLayout->setContentsMargins(20, 24, 20, 24);
    loadingLayout->setSpacing(10);

    m_loadingLabel = new QLabel("⏳ Loading items from campus server...", m_loadingBox);
    m_loadingLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #2563EB;");
    m_loadingLabel->setAlignment(Qt::AlignCenter);

    m_progressBar = new QProgressBar(m_loadingBox);
    m_progressBar->setRange(0, 0); // Indeterminate animated progress bar
    m_progressBar->setFixedWidth(240);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   background-color: #E2EAF4;"
        "   border-radius: 3px;"
        "   border: none;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #2563EB;"
        "   border-radius: 3px;"
        "}"
    );

    loadingLayout->addWidget(m_loadingLabel);
    loadingLayout->addWidget(m_progressBar);
    m_loadingBox->hide();
    rootLayout->addWidget(m_loadingBox);

    // 4. Error State Box
    m_errorBox = new QWidget(this);
    m_errorBox->setStyleSheet(
        "QWidget {"
        "   background-color: #FEF2F2;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 10px;"
        "   padding: 16px;"
        "}"
    );
    QHBoxLayout *errorLayout = new QHBoxLayout(m_errorBox);
    m_errorLabel = new QLabel(m_errorBox);
    m_errorLabel->setStyleSheet("color: #DC2626; font-size: 13px; font-weight: 600; background: transparent;");

    m_retryBtn = new QPushButton("Retry Connection", m_errorBox);
    m_retryBtn->setCursor(Qt::PointingHandCursor);
    m_retryBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #DC2626;"
        "   color: white;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "   padding: 6px 14px;"
        "   border-radius: 6px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #B91C1C; }"
    );
    connect(m_retryBtn, &QPushButton::clicked, this, &BrowseScreen::refreshData);

    errorLayout->addWidget(m_errorLabel);
    errorLayout->addStretch();
    errorLayout->addWidget(m_retryBtn);
    m_errorBox->hide();
    rootLayout->addWidget(m_errorBox);

    // 5. Item Count Header
    m_countLabel = new QLabel("Showing 0 items", this);
    m_countLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #64748B; background: transparent;");
    rootLayout->addWidget(m_countLabel);

    // 6. Scroll Area with Card Grid
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    m_gridContainer = new QWidget(scrollArea);
    m_gridContainer->setStyleSheet("background: transparent;");
    m_cardsGrid = new QGridLayout(m_gridContainer);
    m_cardsGrid->setSpacing(14);
    m_cardsGrid->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(m_gridContainer);
    rootLayout->addWidget(scrollArea);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &BrowseScreen::applyFilters);
    connect(m_locationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BrowseScreen::applyFilters);

    onFilterButtonClicked("all");
}

void BrowseScreen::resetFilters()
{
    m_searchEdit->clear();
    m_locationCombo->setCurrentIndex(0);
    onFilterButtonClicked("all");
}

void BrowseScreen::onFilterButtonClicked(const QString &filter)
{
    m_selectedTypeFilter = filter;

    QString defaultStyle =
        "QPushButton {"
        "   background-color: #F4F8FF;"
        "   color: #172B4D;"
        "   font-weight: 600;"
        "   padding: 8px 14px;"
        "   border-radius: 8px;"
        "   border: 1px solid #E2EAF4;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #EAF2FF; color: #2563EB; }";

    m_allBtn->setStyleSheet(defaultStyle);
    m_lostBtn->setStyleSheet(defaultStyle);
    m_foundBtn->setStyleSheet(defaultStyle);

    if (filter == "all") {
        m_allBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #2563EB;"
            "   color: #FFFFFF;"
            "   font-weight: 700;"
            "   padding: 8px 14px;"
            "   border-radius: 8px;"
            "   border: none;"
            "   font-size: 12px;"
            "}"
        );
    } else if (filter == "lost") {
        m_lostBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #EF4444;"
            "   color: #FFFFFF;"
            "   font-weight: 700;"
            "   padding: 8px 14px;"
            "   border-radius: 8px;"
            "   border: none;"
            "   font-size: 12px;"
            "}"
        );
    } else if (filter == "found") {
        m_foundBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #16A66A;"
            "   color: #FFFFFF;"
            "   font-weight: 700;"
            "   padding: 8px 14px;"
            "   border-radius: 8px;"
            "   border: none;"
            "   font-size: 12px;"
            "}"
        );
    }

    applyFilters();
}

void BrowseScreen::showLoading(bool loading)
{
    m_loadingBox->setVisible(loading);
    if (loading) {
        m_errorBox->hide();
    }
}

void BrowseScreen::showError(const QString &errorMessage)
{
    m_loadingBox->hide();
    m_errorLabel->setText("❌ " + errorMessage);
    m_errorBox->show();
}

void BrowseScreen::refreshData()
{
    showLoading(true);
    m_errorBox->hide();

    m_apiClient->getAllItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            showLoading(false);
            m_allItems = items;
            applyFilters();
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            showLoading(false);
            showError("Unable to fetch catalog: " + errorMessage);
        }
    );
}

void BrowseScreen::applyFilters()
{
    QString search = m_searchEdit->text().trimmed().toLower();
    QString locFilter = m_locationCombo->currentData().toString().toLower();

    QVector<Item> filtered;

    for (const Item &item : m_allItems) {
        // 1. Status Filter (All, Lost, Found)
        if (m_selectedTypeFilter == "lost" && !item.isLost()) continue;
        if (m_selectedTypeFilter == "found" && !item.isFound()) continue;

        // 2. Location Filter
        if (!locFilter.isEmpty() && !item.location.toLower().contains(locFilter)) continue;

        // 3. Search Bar Filter (Title, Description, Location)
        if (!search.isEmpty()) {
            bool matches = item.title.toLower().contains(search) ||
                           item.description.toLower().contains(search) ||
                           item.location.toLower().contains(search);
            if (!matches) continue;
        }

        filtered.append(item);
    }

    renderItems(filtered);
}

void BrowseScreen::renderItems(const QVector<Item> &items)
{
    m_countLabel->setText(QString("Showing %1 %2")
        .arg(items.size())
        .arg(items.size() == 1 ? "item" : "items"));

    // Clear previous items in grid safely
    QLayoutItem *child;
    while ((child = m_cardsGrid->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // Empty State
    if (items.isEmpty()) {
        QString emptyTitle = m_allItems.isEmpty() ? "No Items Reported Yet" : "No items found";
        QString emptySub = m_allItems.isEmpty() 
            ? "There are currently no lost or found items reported on campus."
            : "Try changing your search or filters.";

        EmptyStateWidget *empty = new EmptyStateWidget(
            "🔍",
            emptyTitle,
            emptySub,
            "+ Report an Item",
            m_gridContainer
        );
        connect(empty, &EmptyStateWidget::actionClicked, this, [this]() { emit navigateTo(2); }); // 2 = Report
        m_cardsGrid->addWidget(empty, 0, 0, 1, 2);
        return;
    }

    // Render Item Cards (2-column responsive grid)
    for (int i = 0; i < items.size(); ++i) {
        const Item &item = items[i];
        ItemCardWidget *card = new ItemCardWidget(item, m_gridContainer);
        connect(card, &ItemCardWidget::viewDetailsRequested, this, &BrowseScreen::viewItemDetails);
        connect(card, &ItemCardWidget::deleteItemRequested, this, &BrowseScreen::confirmDeleteItem);
        m_cardsGrid->addWidget(card, i / 2, i % 2);
    }
}

void BrowseScreen::confirmDeleteItem(const Item &item)
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

    showLoading(true);
    m_apiClient->deleteItem(
        item.id,
        [this, item](const QJsonObject &response) {
            Q_UNUSED(response);
            showLoading(false);
            emit showToast(QString("Item \"%1\" deleted successfully.").arg(item.title), ToastType::Success);
            refreshData();
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            showLoading(false);
            QMessageBox::critical(
                this,
                "Delete Failed",
                QString("Unable to delete item: %1").arg(errorMessage)
            );
        }
    );
}