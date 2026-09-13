#include "DashboardScreen.h"
#include "../auth/AuthManager.h"
#include "../components/ItemCardWidget.h"
#include "../components/EmptyStateWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QTime>
#include <QGraphicsDropShadowEffect>

DashboardScreen::DashboardScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_greetingLabel(nullptr)
    , m_subGreetingLabel(nullptr)
    , m_emailLabel(nullptr)
    , m_roleBadge(nullptr)
    , m_statTotalVal(nullptr)
    , m_statLostVal(nullptr)
    , m_statFoundVal(nullptr)
    , m_statResolvedVal(nullptr)
    , m_itemCountLabel(nullptr)
    , m_itemsContainer(nullptr)
    , m_itemsGrid(nullptr)
    , m_pickupAlertBanner(nullptr)
    , m_pickupAlertText(nullptr)
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
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(32, 24, 32, 32);
    rootLayout->setSpacing(20);

    // 1. Hero Section Card
    QFrame *heroCard = new QFrame(this);
    heroCard->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid rgba(226, 232, 240, 0.95);"
        "   border-radius: 16px;"
        "   padding: 20px 24px;"
        "}"
    );

    QHBoxLayout *heroLayout = new QHBoxLayout(heroCard);
    heroLayout->setContentsMargins(4, 4, 4, 4);
    heroLayout->setSpacing(24);

    // Left side: Greeting + Subtitle + Action buttons
    QVBoxLayout *heroTextLayout = new QVBoxLayout();
    heroTextLayout->setSpacing(8);

    QHBoxLayout *nameRow = new QHBoxLayout();
    nameRow->setSpacing(10);

    m_greetingLabel = new QLabel(this);
    m_greetingLabel->setStyleSheet("font-size: 22px; font-weight: 800; color: #0F172A; background: transparent;");

    m_roleBadge = new QLabel(this);
    m_roleBadge->setStyleSheet("font-size: 11px; font-weight: 800; padding: 3px 10px; border-radius: 6px; background-color: #EFF6FF; color: #1D4ED8; border: 1px solid #BFDBFE;");

    nameRow->addWidget(m_greetingLabel);
    nameRow->addWidget(m_roleBadge);
    nameRow->addStretch();

    m_subGreetingLabel = new QLabel(
        "Welcome to your personal campus recovery desk. Track your reports, verify claims, and browse active listings.",
        this
    );
    m_subGreetingLabel->setStyleSheet("font-size: 13px; color: #64748B; font-weight: 500; background: transparent;");
    m_subGreetingLabel->setWordWrap(true);

    // Quick Hero Action Buttons
    QHBoxLayout *heroButtons = new QHBoxLayout();
    heroButtons->setSpacing(10);

    QPushButton *reportLostBtn = new QPushButton("+ Report Lost Item", heroCard);
    reportLostBtn->setCursor(Qt::PointingHandCursor);
    reportLostBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #EF4444;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "   padding: 9px 16px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #DC2626; }"
    );
    connect(reportLostBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });

    QPushButton *reportFoundBtn = new QPushButton("+ Report Found Item", heroCard);
    reportFoundBtn->setCursor(Qt::PointingHandCursor);
    reportFoundBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #10B981;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "   padding: 9px 16px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #059669; }"
    );
    connect(reportFoundBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });

    QPushButton *browseFeedBtn = new QPushButton("🔍 Browse Items", heroCard);
    browseFeedBtn->setCursor(Qt::PointingHandCursor);
    browseFeedBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "   padding: 9px 16px;"
        "   border-radius: 8px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(browseFeedBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); });

    heroButtons->addWidget(reportLostBtn);
    heroButtons->addWidget(reportFoundBtn);
    heroButtons->addWidget(browseFeedBtn);
    heroButtons->addStretch();

    heroTextLayout->addLayout(nameRow);
    heroTextLayout->addWidget(m_subGreetingLabel);
    heroTextLayout->addSpacing(4);
    heroTextLayout->addLayout(heroButtons);

    heroLayout->addLayout(heroTextLayout, 3);

    // Right side: Campus Identity Visual Tile
    QFrame *campusTile = new QFrame(heroCard);
    campusTile->setFixedWidth(200);
    campusTile->setStyleSheet(
        "QFrame {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 12px;"
        "}"
    );
    QVBoxLayout *tileLayout = new QVBoxLayout(campusTile);
    tileLayout->setAlignment(Qt::AlignCenter);
    tileLayout->setSpacing(4);

    QLabel *tileIcon = new QLabel("🏛️", campusTile);
    tileIcon->setStyleSheet("font-size: 28px; background: transparent;");
    tileIcon->setAlignment(Qt::AlignCenter);

    QLabel *tileTitle = new QLabel("GEHU Bhimtal", campusTile);
    tileTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #0F172A; background: transparent;");
    tileTitle->setAlignment(Qt::AlignCenter);

    QLabel *tileSub = new QLabel("Campus Portal Network", campusTile);
    tileSub->setStyleSheet("font-size: 10px; color: #64748B; font-weight: 600; background: transparent;");
    tileSub->setAlignment(Qt::AlignCenter);

    tileLayout->addWidget(tileIcon);
    tileLayout->addWidget(tileTitle);
    tileLayout->addWidget(tileSub);

    heroLayout->addWidget(campusTile, 1);
    rootLayout->addWidget(heroCard);

    // Urgent Pickup Alert Banner (Shown when user has approved claims)
    m_pickupAlertBanner = new QFrame(this);
    m_pickupAlertBanner->setStyleSheet(
        "QFrame {"
        "   background-color: #ECFDF5;"
        "   border: 1.5px solid #059669;"
        "   border-radius: 12px;"
        "   padding: 12px 18px;"
        "}"
    );
    QHBoxLayout *alertLayout = new QHBoxLayout(m_pickupAlertBanner);
    alertLayout->setContentsMargins(0, 0, 0, 0);
    alertLayout->setSpacing(12);

    QLabel *alertIcon = new QLabel("🎫", m_pickupAlertBanner);
    alertIcon->setStyleSheet("font-size: 22px; background: transparent;");
    alertLayout->addWidget(alertIcon);

    m_pickupAlertText = new QLabel(m_pickupAlertBanner);
    m_pickupAlertText->setStyleSheet("font-size: 13px; font-weight: 700; color: #065F46; background: transparent;");
    alertLayout->addWidget(m_pickupAlertText, 1);

    QPushButton *viewPassBtn = new QPushButton("View Pickup Pass →", m_pickupAlertBanner);
    viewPassBtn->setCursor(Qt::PointingHandCursor);
    viewPassBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #059669;"
        "   color: #FFFFFF;"
        "   font-size: 12px;"
        "   font-weight: 800;"
        "   padding: 8px 16px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #047857; }"
    );
    connect(viewPassBtn, &QPushButton::clicked, this, [this]() {
        emit navigateTo(8); // Navigate to My Claims
    });
    alertLayout->addWidget(viewPassBtn);
    m_pickupAlertBanner->hide();
    rootLayout->addWidget(m_pickupAlertBanner);

    // 2. 4 KPI Metrics Row
    QHBoxLayout *kpiLayout = new QHBoxLayout();
    kpiLayout->setSpacing(14);

    m_statTotalVal = new QLabel("0", this);
    m_statLostVal = new QLabel("0", this);
    m_statFoundVal = new QLabel("0", this);
    m_statResolvedVal = new QLabel("0", this);

    kpiLayout->addWidget(createKpiCard("📋", "TOTAL REPORTS", m_statTotalVal, "#4F46E5"));
    kpiLayout->addWidget(createKpiCard("🔴", "LOST REPORTED", m_statLostVal, "#EF4444"));
    kpiLayout->addWidget(createKpiCard("🟢", "FOUND REPORTED", m_statFoundVal, "#10B981"));
    kpiLayout->addWidget(createKpiCard("✅", "RESOLVED / CLAIMED", m_statResolvedVal, "#8B5CF6"));

    rootLayout->addLayout(kpiLayout);

    // 3. Quick Actions Cards Row
    QHBoxLayout *quickRow = new QHBoxLayout();
    quickRow->setSpacing(14);

    quickRow->addWidget(createQuickActionCard(
        "📢",
        "Report Lost Possession",
        "Misplaced a phone, wallet, student ID, or books? Log a report right away.",
        2
    ));
    quickRow->addWidget(createQuickActionCard(
        "🎁",
        "Report Found Possession",
        "Discovered an unattended item on campus grounds? Report it for recovery.",
        2
    ));
    quickRow->addWidget(createQuickActionCard(
        "🔎",
        "Search Campus Inventory",
        "Browse the full university register of active lost and discovered items.",
        1
    ));
    quickRow->addWidget(createQuickActionCard(
        "🎒",
        "My Claims & Passes",
        "Track ownership claims, verification status, and access digital pickup passes.",
        8
    ));

    rootLayout->addLayout(quickRow);

    // 4. Section Header for Recent Reports
    QHBoxLayout *secHeader = new QHBoxLayout();
    QLabel *sectionTitle = new QLabel("My Recent Reports", this);
    sectionTitle->setStyleSheet("font-size: 17px; font-weight: 800; color: #0F172A; background: transparent;");
    
    m_itemCountLabel = new QLabel("0 items", this);
    m_itemCountLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748B; background: transparent;");

    secHeader->addWidget(sectionTitle);
    secHeader->addStretch();
    secHeader->addWidget(m_itemCountLabel);
    rootLayout->addLayout(secHeader);

    // 5. Scroll Area with My Reports Grid
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    m_itemsContainer = new QWidget(this);
    m_itemsContainer->setStyleSheet("background: transparent;");
    m_itemsGrid = new QGridLayout(m_itemsContainer);
    m_itemsGrid->setSpacing(16);
    m_itemsGrid->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(m_itemsContainer);
    rootLayout->addWidget(scrollArea, 1);
}

QFrame* DashboardScreen::createKpiCard(
    const QString &icon,
    const QString &label,
    QLabel *valLabel,
    const QString &accentColor)
{
    QFrame *card = new QFrame();
    card->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid rgba(226, 232, 240, 0.95);"
        "   border-radius: 14px;"
        "}"
    );

    QHBoxLayout *l = new QHBoxLayout(card);
    l->setContentsMargins(16, 14, 16, 14);
    l->setSpacing(14);

    // Colored Icon Container
    QFrame *iconBox = new QFrame(card);
    iconBox->setFixedSize(44, 44);
    iconBox->setStyleSheet(QString(
        "QFrame {"
        "   background-color: %1;"
        "   border-radius: 10px;"
        "   border: none;"
        "}"
    ).arg(accentColor + "18")); // ~10% opacity

    QHBoxLayout *ibLayout = new QHBoxLayout(iconBox);
    ibLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *iLbl = new QLabel(icon, iconBox);
    iLbl->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    iLbl->setAlignment(Qt::AlignCenter);
    ibLayout->addWidget(iLbl);

    QVBoxLayout *tL = new QVBoxLayout();
    tL->setSpacing(2);

    valLabel->setStyleSheet(QString("font-size: 22px; font-weight: 800; color: %1; background: transparent; border: none;").arg(accentColor));
    QLabel *sub = new QLabel(label, card);
    sub->setStyleSheet("font-size: 10px; font-weight: 700; color: #64748B; letter-spacing: 0.5px; background: transparent; border: none;");

    tL->addWidget(valLabel);
    tL->addWidget(sub);

    l->addWidget(iconBox);
    l->addLayout(tL);
    l->addStretch();
    return card;
}

QFrame* DashboardScreen::createQuickActionCard(
    const QString &icon,
    const QString &title,
    const QString &desc,
    int targetScreen)
{
    QFrame *card = new QFrame();
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.90);"
        "   border: 1px solid rgba(226, 232, 240, 0.95);"
        "   border-radius: 12px;"
        "   padding: 14px;"
        "}"
        "QFrame:hover {"
        "   background-color: #FFFFFF;"
        "   border-color: #CBD5E1;"
        "}"
    );

    QVBoxLayout *l = new QVBoxLayout(card);
    l->setContentsMargins(12, 10, 12, 10);
    l->setSpacing(6);

    QHBoxLayout *headerRow = new QHBoxLayout();
    headerRow->setSpacing(8);

    QLabel *iLbl = new QLabel(icon, card);
    iLbl->setStyleSheet("font-size: 18px; background: transparent;");

    QLabel *tLbl = new QLabel(title, card);
    tLbl->setStyleSheet("font-size: 13px; font-weight: 800; color: #0F172A; background: transparent;");

    headerRow->addWidget(iLbl);
    headerRow->addWidget(tLbl);
    headerRow->addStretch();

    QLabel *dLbl = new QLabel(desc, card);
    dLbl->setStyleSheet("font-size: 11px; color: #64748B; line-height: 1.3; background: transparent;");
    dLbl->setWordWrap(true);

    l->addLayout(headerRow);
    l->addWidget(dLbl);

    // Make frame clickable via mousePressEvent
    card->installEventFilter(this);
    // Use an action button or property
    QPushButton *invBtn = new QPushButton("Open →", card);
    invBtn->setCursor(Qt::PointingHandCursor);
    invBtn->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #4F46E5;"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   text-align: left;"
        "   border: none;"
        "   padding: 2px 0px;"
        "}"
        "QPushButton:hover { color: #4338CA; text-decoration: underline; }"
    );
    connect(invBtn, &QPushButton::clicked, this, [this, targetScreen]() {
        emit navigateTo(targetScreen);
    });
    l->addWidget(invBtn);

    return card;
}

void DashboardScreen::updateProfile()
{
    User user = AuthManager::instance().currentUser();
    QString name = user.name.isEmpty() ? "Student" : user.name.trimmed();
    m_greetingLabel->setText(QString("%1, %2 👋").arg(getGreeting(), name));

    if (user.isAdmin()) {
        m_roleBadge->setText("👑 ADMINISTRATOR");
        m_roleBadge->setStyleSheet("font-size: 10px; font-weight: 800; padding: 3px 8px; border-radius: 6px; background-color: #F5F3FF; color: #7C3AED; border: 1px solid #DDD6FE;");
    } else {
        m_roleBadge->setText("🎓 STUDENT");
        m_roleBadge->setStyleSheet("font-size: 10px; font-weight: 800; padding: 3px 8px; border-radius: 6px; background-color: #EFF6FF; color: #1D4ED8; border: 1px solid #BFDBFE;");
    }
}

void DashboardScreen::refreshData()
{
    updateProfile();

    m_apiClient->getUserItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            renderUserItems(items);
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            Q_UNUSED(errorMessage);
            // Fallback to getAllItems if getUserItems fails
            m_apiClient->getAllItems(
                [this](const QVector<Item> &items, int count) {
                    Q_UNUSED(count);
                    renderUserItems(items);
                },
                nullptr
            );
        }
    );

    // Query active claims to show urgent pickup banner if approved
    m_apiClient->getUserClaims(
        [this](const QVector<Claim> &claims, int count) {
            Q_UNUSED(count);
            int approvedCount = 0;
            QString firstRef;
            QString firstLoc;
            for (const Claim &c : claims) {
                if (c.isApproved()) {
                    approvedCount++;
                    if (firstRef.isEmpty()) {
                        firstRef = c.claimReference;
                        firstLoc = c.pickupLocation.isEmpty() ? "Admin Block - Room 102" : c.pickupLocation;
                    }
                }
            }

            if (approvedCount > 0 && m_pickupAlertBanner && m_pickupAlertText) {
                m_pickupAlertText->setText(QString("🎉 You have %1 approved item ready for physical pickup at %2 (Ref: %3)")
                    .arg(approvedCount == 1 ? "an" : QString::number(approvedCount))
                    .arg(firstLoc)
                    .arg(firstRef));
                m_pickupAlertBanner->show();
            } else if (m_pickupAlertBanner) {
                m_pickupAlertBanner->hide();
            }
        },
        nullptr
    );
}

void DashboardScreen::renderUserItems(const QVector<Item> &items)
{
    int currentUserId = AuthManager::instance().currentUser().id;

    QVector<Item> userItems;
    int lostCount = 0;
    int foundCount = 0;
    int resolvedCount = 0;

    for (const Item &item : items) {
        if (item.userId == currentUserId || items.size() <= 20) {
            // Note: if returned by getUserItems, all items belong to this user
            if (item.userId == currentUserId) {
                userItems.append(item);
                if (item.status.toLower() == "claimed" || item.status.toLower() == "returned") {
                    resolvedCount++;
                } else if (item.isLost()) {
                    lostCount++;
                } else {
                    foundCount++;
                }
            }
        }
    }

    m_statTotalVal->setText(QString::number(userItems.size()));
    m_statLostVal->setText(QString::number(lostCount));
    m_statFoundVal->setText(QString::number(foundCount));
    m_statResolvedVal->setText(QString::number(resolvedCount));

    m_itemCountLabel->setText(QString("Showing %1 %2")
        .arg(userItems.size())
        .arg(userItems.size() == 1 ? "report" : "reports"));

    // Clear previous items safely
    QLayoutItem *child;
    while ((child = m_itemsGrid->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    if (userItems.isEmpty()) {
        EmptyStateWidget *empty = new EmptyStateWidget(
            "📋",
            "No Reports Submitted Yet",
            "You haven't reported any lost or found items yet. Click below to submit your first report.",
            "+ Report an Item",
            m_itemsContainer
        );
        connect(empty, &EmptyStateWidget::actionClicked, this, [this]() { emit navigateTo(2); });
        m_itemsGrid->addWidget(empty, 0, 0, 1, 2);
        return;
    }

    for (int i = 0; i < userItems.size(); ++i) {
        const Item &item = userItems[i];
        ItemCardWidget *card = new ItemCardWidget(item, m_itemsContainer);
        connect(card, &ItemCardWidget::viewDetailsRequested, this, &DashboardScreen::viewItemDetails);
        connect(card, &ItemCardWidget::deleteItemRequested, this, &DashboardScreen::confirmDeleteItem);
        m_itemsGrid->addWidget(card, i / 2, i % 2);
    }
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