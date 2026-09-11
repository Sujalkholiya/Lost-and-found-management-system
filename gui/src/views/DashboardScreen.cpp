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

DashboardScreen::DashboardScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_greetingLabel(new QLabel(this))
    , m_emailLabel(new QLabel(this))
    , m_roleBadge(new QLabel(this))
    , m_statTotalVal(new QLabel("0", this))
    , m_statLostVal(new QLabel("0", this))
    , m_statFoundVal(new QLabel("0", this))
    , m_itemCountLabel(new QLabel("0 items reported", this))
    , m_itemsContainer(new QWidget(this))
    , m_itemsGrid(new QGridLayout(m_itemsContainer))
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(36, 28, 36, 36);
    rootLayout->setSpacing(20);

    // 1. Profile Banner Card
    QFrame *profileCard = new QFrame(this);
    profileCard->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "   padding: 24px;"
        "}"
    );
    QHBoxLayout *profLayout = new QHBoxLayout(profileCard);
    profLayout->setSpacing(20);

    QLabel *avatar = new QLabel("👤", profileCard);
    avatar->setFixedSize(56, 56);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background-color: #EEF2FF; border-radius: 28px; font-size: 28px;");

    QVBoxLayout *profInfo = new QVBoxLayout();
    profInfo->setSpacing(4);

    QHBoxLayout *nameRow = new QHBoxLayout();
    m_greetingLabel->setStyleSheet("font-size: 20px; font-weight: 800; color: #0F172A;");
    m_roleBadge->setStyleSheet("font-size: 11px; font-weight: 800; padding: 3px 8px; border-radius: 6px; background-color: #EFF6FF; color: #1D4ED8; border: 1px solid #BFDBFE;");
    nameRow->addWidget(m_greetingLabel);
    nameRow->addWidget(m_roleBadge);
    nameRow->addStretch();

    m_emailLabel->setStyleSheet("font-size: 13px; color: #64748B; font-weight: 500;");

    profInfo->addLayout(nameRow);
    profInfo->addWidget(m_emailLabel);

    // Quick Actions in Profile Header
    QHBoxLayout *quickActionsLayout = new QHBoxLayout();
    quickActionsLayout->setSpacing(10);

    QPushButton *browseBtn = new QPushButton("🔍 Browse Items", profileCard);
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "   padding: 8px 14px;"
        "   border-radius: 8px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(browseBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); });

    QPushButton *reportBtn = new QPushButton("➕ Report an Item", profileCard);
    reportBtn->setCursor(Qt::PointingHandCursor);
    reportBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: white;"
        "   font-weight: 800;"
        "   font-size: 12px;"
        "   padding: 8px 16px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(reportBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });

    QPushButton *logoutBtn = new QPushButton("🚪 Log Out", profileCard);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #DC2626;"
        "   font-weight: 700;"
        "   font-size: 12px;"
        "   padding: 8px 12px;"
        "   border-radius: 8px;"
        "   border: 1px solid #FECDD3;"
        "}"
        "QPushButton:hover { background-color: #FEE2E2; }"
    );
    connect(logoutBtn, &QPushButton::clicked, &AuthManager::instance(), &AuthManager::logout);

    quickActionsLayout->addWidget(browseBtn);
    quickActionsLayout->addWidget(reportBtn);
    quickActionsLayout->addWidget(logoutBtn);

    profLayout->addWidget(avatar);
    profLayout->addLayout(profInfo);
    profLayout->addStretch();
    profLayout->addLayout(quickActionsLayout);

    rootLayout->addWidget(profileCard);

    // 2. Statistics Summary Row
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    auto createStatPill = [](const QString &icon, const QString &label, QLabel *valLabel, const QString &color) -> QFrame* {
        QFrame *card = new QFrame();
        card->setStyleSheet(
            "QFrame {"
            "   background-color: #FFFFFF;"
            "   border: 1px solid #E2E8F0;"
            "   border-radius: 12px;"
            "   padding: 14px 18px;"
            "}"
        );
        QHBoxLayout *l = new QHBoxLayout(card);
        l->setContentsMargins(12, 10, 12, 10);
        l->setSpacing(12);

        QLabel *iLbl = new QLabel(icon, card);
        iLbl->setStyleSheet("font-size: 20px; background: transparent;");

        QVBoxLayout *tL = new QVBoxLayout();
        tL->setSpacing(2);
        valLabel->setStyleSheet(QString("font-size: 18px; font-weight: 800; color: %1;").arg(color));
        QLabel *sub = new QLabel(label, card);
        sub->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748B;");

        tL->addWidget(valLabel);
        tL->addWidget(sub);

        l->addWidget(iLbl);
        l->addLayout(tL);
        l->addStretch();
        return card;
    };

    statsLayout->addWidget(createStatPill("📋", "Total My Reports", m_statTotalVal, "#4F46E5"));
    statsLayout->addWidget(createStatPill("🔴", "Lost Items Reported", m_statLostVal, "#DC2626"));
    statsLayout->addWidget(createStatPill("🟢", "Found Items Reported", m_statFoundVal, "#059669"));

    rootLayout->addLayout(statsLayout);

    // 3. Section Header
    QHBoxLayout *secHeader = new QHBoxLayout();
    QLabel *sectionTitle = new QLabel("My Reported Items", this);
    sectionTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: #0F172A; background: transparent;");
    m_itemCountLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #64748B; background: transparent;");
    secHeader->addWidget(sectionTitle);
    secHeader->addStretch();
    secHeader->addWidget(m_itemCountLabel);
    rootLayout->addLayout(secHeader);

    // 4. Items list inside ScrollArea
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    m_itemsContainer->setStyleSheet("background: transparent;");
    m_itemsGrid->setSpacing(16);
    m_itemsGrid->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(m_itemsContainer);
    rootLayout->addWidget(scrollArea);
}

void DashboardScreen::updateProfile()
{
    User user = AuthManager::instance().currentUser();
    m_greetingLabel->setText(QString("Welcome, %1!").arg(user.name.isEmpty() ? "Student" : user.name));
    m_emailLabel->setText(user.email);
    m_roleBadge->setText(user.isAdmin() ? "👑 ADMINISTRATOR" : "🎒 STUDENT");
}

void DashboardScreen::refreshData()
{
    updateProfile();

    m_apiClient->getAllItems(
        [this](const QVector<Item> &items, int count) {
            Q_UNUSED(count);
            renderUserItems(items);
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            Q_UNUSED(errorMessage);
        }
    );
}

void DashboardScreen::renderUserItems(const QVector<Item> &items)
{
    int currentUserId = AuthManager::instance().currentUser().id;

    QVector<Item> userItems;
    int lostCount = 0;
    int foundCount = 0;

    for (const Item &item : items) {
        if (item.userId == currentUserId) {
            userItems.append(item);
            if (item.isLost()) {
                lostCount++;
            } else {
                foundCount++;
            }
        }
    }

    m_statTotalVal->setText(QString::number(userItems.size()));
    m_statLostVal->setText(QString::number(lostCount));
    m_statFoundVal->setText(QString::number(foundCount));

    m_itemCountLabel->setText(QString("Showing %1 %2")
        .arg(userItems.size())
        .arg(userItems.size() == 1 ? "item" : "items"));

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