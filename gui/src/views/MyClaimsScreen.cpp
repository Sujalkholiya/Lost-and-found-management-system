#include "MyClaimsScreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>
#include "../network/ImageLoader.h"
#include "../auth/AuthManager.h"

MyClaimsScreen::MyClaimsScreen(ApiClient* apiClient, QWidget* parent)
    : QWidget(parent),
      m_apiClient(apiClient),
      m_currentFilter("all")
{
    setupUi();
}

void MyClaimsScreen::setupUi()
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(32, 28, 32, 28);
    rootLayout->setSpacing(20);

    // 1. Top Header Row
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    QVBoxLayout* titleBox = new QVBoxLayout();
    titleBox->setSpacing(4);

    QLabel* pageTitle = new QLabel("🎒 My Item Claims & Pickup Passes", this);
    pageTitle->setStyleSheet("font-size: 24px; font-weight: 800; color: #0F172A;");
    titleBox->addWidget(pageTitle);

    QLabel* pageSubtitle = new QLabel(
        "Track live verification of your claimed items, view official pickup passes, and claim history.",
        this
    );
    pageSubtitle->setStyleSheet("font-size: 13px; color: #64748B;");
    titleBox->addWidget(pageSubtitle);

    headerLayout->addLayout(titleBox, 1);

    m_refreshBtn = new QPushButton("🔄 Refresh", this);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   color: #334155;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 8px 16px;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: #94A3B8; }"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, &MyClaimsScreen::loadClaims);
    headerLayout->addWidget(m_refreshBtn);

    rootLayout->addLayout(headerLayout);

    // 2. Filter Tabs & Counter Row
    QHBoxLayout* filterRow = new QHBoxLayout();
    filterRow->setSpacing(8);

    auto createFilterBtn = [this](const QString& text, const QString& filter) {
        QPushButton* btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("filterValue", filter);
        connect(btn, &QPushButton::clicked, this, [this, filter]() {
            onFilterClicked(filter);
        });
        return btn;
    };

    m_btnAll = createFilterBtn("All Claims", "all");
    m_btnPending = createFilterBtn("Pending Review", "pending");
    m_btnApproved = createFilterBtn("Ready for Pickup", "approved");
    m_btnHandedOver = createFilterBtn("Handed Over", "handed_over");
    m_btnRejected = createFilterBtn("Rejected", "rejected");

    filterRow->addWidget(m_btnAll);
    filterRow->addWidget(m_btnPending);
    filterRow->addWidget(m_btnApproved);
    filterRow->addWidget(m_btnHandedOver);
    filterRow->addWidget(m_btnRejected);
    filterRow->addStretch();

    m_countLabel = new QLabel("0 claims found", this);
    m_countLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #64748B;");
    filterRow->addWidget(m_countLabel);

    rootLayout->addLayout(filterRow);

    // 3. Scrollable Claims Container
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    m_cardsContainer = new QWidget(scrollArea);
    m_cardsContainer->setStyleSheet("background: transparent;");
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(0, 8, 0, 16);
    m_cardsLayout->setSpacing(16);

    // Loading Widget
    m_loadingWidget = new QWidget(m_cardsContainer);
    QVBoxLayout* loadingLayout = new QVBoxLayout(m_loadingWidget);
    loadingLayout->setAlignment(Qt::AlignCenter);
    QLabel* loadingText = new QLabel("⏳ Loading your claims from server...", m_loadingWidget);
    loadingText->setStyleSheet("font-size: 14px; color: #64748B; font-weight: 600; padding: 40px;");
    loadingLayout->addWidget(loadingText);
    m_loadingWidget->hide();
    m_cardsLayout->addWidget(m_loadingWidget);

    // Empty State Widget
    m_emptyWidget = new QWidget(m_cardsContainer);
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_emptyWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);
    emptyLayout->setSpacing(12);
    QLabel* emptyIcon = new QLabel("📦", m_emptyWidget);
    emptyIcon->setStyleSheet("font-size: 48px;");
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyIcon);

    QLabel* emptyTitle = new QLabel("No claims in this category", m_emptyWidget);
    emptyTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #334155;");
    emptyTitle->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyTitle);

    QLabel* emptyDesc = new QLabel(
        "When you find your lost property among reported items and click 'Claim Item', you can track review progress and access your official pickup pass here.",
        m_emptyWidget
    );
    emptyDesc->setStyleSheet("font-size: 13px; color: #64748B; line-height: 140%; max-width: 480px;");
    emptyDesc->setWordWrap(true);
    emptyDesc->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyDesc);

    QPushButton* browseBtn = new QPushButton("Browse Found Items", m_emptyWidget);
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 22px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        emit navigateTo(1); // Browse screen index
    });
    emptyLayout->addWidget(browseBtn, 0, Qt::AlignCenter);

    m_emptyWidget->hide();
    m_cardsLayout->addWidget(m_emptyWidget);

    m_cardsLayout->addStretch();
    scrollArea->setWidget(m_cardsContainer);
    rootLayout->addWidget(scrollArea, 1);

    onFilterClicked("all");
}

void MyClaimsScreen::refreshData()
{
    loadClaims();
}

void MyClaimsScreen::onFilterClicked(const QString& filter)
{
    m_currentFilter = filter;

    auto updateBtnStyle = [this](QPushButton* btn, bool active) {
        if (active) {
            btn->setStyleSheet(
                "QPushButton {"
                "   background-color: #2563EB;"
                "   color: #FFFFFF;"
                "   border: none;"
                "   border-radius: 20px;"
                "   font-size: 12px;"
                "   font-weight: 700;"
                "   padding: 7px 18px;"
                "}"
            );
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "   background-color: #F1F5F9;"
                "   color: #475569;"
                "   border: 1px solid #E2E8F0;"
                "   border-radius: 20px;"
                "   font-size: 12px;"
                "   font-weight: 600;"
                "   padding: 7px 18px;"
                "}"
                "QPushButton:hover { background-color: #E2E8F0; }"
            );
        }
    };

    updateBtnStyle(m_btnAll, filter == "all");
    updateBtnStyle(m_btnPending, filter == "pending");
    updateBtnStyle(m_btnApproved, filter == "approved");
    updateBtnStyle(m_btnHandedOver, filter == "handed_over");
    updateBtnStyle(m_btnRejected, filter == "rejected");

    renderClaims();
}

void MyClaimsScreen::loadClaims()
{
    if (!m_apiClient || !AuthManager::instance().isLoggedIn()) {
        m_allClaims.clear();
        renderClaims();
        return;
    }

    m_loadingWidget->show();
    m_emptyWidget->hide();

    m_apiClient->getUserClaims(
        [this](const QVector<Claim>& claims, int count) {
            Q_UNUSED(count);
            m_loadingWidget->hide();
            m_allClaims = claims;
            renderClaims();
        },
        [this](int statusCode, const QString& errorMessage) {
            Q_UNUSED(statusCode);
            m_loadingWidget->hide();
            emit showToast(QString("Failed to load claims: %1").arg(errorMessage), ToastType::Error);
            renderClaims();
        }
    );
}

void MyClaimsScreen::renderClaims()
{
    // Clear previously added claim cards
    QLayoutItem* item;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_loadingWidget && item->widget() != m_emptyWidget) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    QVector<Claim> filtered;
    for (const Claim& c : m_allClaims) {
        if (m_currentFilter == "all" || c.status.compare(m_currentFilter, Qt::CaseInsensitive) == 0) {
            filtered.append(c);
        }
    }

    m_countLabel->setText(QString("%1 claim%2").arg(filtered.size()).arg(filtered.size() == 1 ? "" : "s"));

    if (filtered.isEmpty()) {
        m_emptyWidget->show();
        m_cardsLayout->addWidget(m_emptyWidget);
        m_cardsLayout->addStretch();
        return;
    }

    m_emptyWidget->hide();

    for (const Claim& c : filtered) {
        QWidget* card = createClaimCard(c);
        m_cardsLayout->addWidget(card);
    }

    m_cardsLayout->addStretch();
}

QWidget* MyClaimsScreen::createClaimCard(const Claim& claim)
{
    QFrame* card = new QFrame(m_cardsContainer);
    card->setStyleSheet(
        "QFrame#claimCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 14px;"
        "}"
    );
    card->setObjectName("claimCard");

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 18);
    cardLayout->setSpacing(14);

    // 1. Card Top Bar: Reference + Badge + Date
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->setSpacing(10);

    QLabel* refBadge = new QLabel(QString("🏷️ %1").arg(claim.claimReference), card);
    refBadge->setStyleSheet(
        "background-color: #F1F5F9; color: #0F172A; font-family: monospace; font-size: 13px; font-weight: 800; padding: 5px 10px; border-radius: 6px; border: 1px solid #CBD5E1;"
    );
    topBar->addWidget(refBadge);

    QLabel* statusBadge = new QLabel(claim.statusDisplayName(), card);
    statusBadge->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 12px; font-weight: bold; padding: 5px 12px; border-radius: 6px; border: 1px solid %2;"
    ).arg(claim.statusBadgeBg(), claim.statusColor()));
    topBar->addWidget(statusBadge);

    topBar->addStretch();

    if (!claim.createdAt.isEmpty()) {
        QLabel* dateLabel = new QLabel(QString("🕒 Submitted: %1").arg(claim.createdAt), card);
        dateLabel->setStyleSheet("font-size: 12px; color: #64748B;");
        topBar->addWidget(dateLabel);
    }
    cardLayout->addLayout(topBar);

    // 2. Item Information Row
    QHBoxLayout* itemRow = new QHBoxLayout();
    itemRow->setSpacing(14);

    if (claim.hasImage()) {
        QLabel* imgLabel = new QLabel(card);
        imgLabel->setFixedSize(68, 68);
        imgLabel->setStyleSheet("background-color: #F1F5F9; border-radius: 8px; border: 1px solid #CBD5E1;");
        imgLabel->setAlignment(Qt::AlignCenter);
        ImageLoader::instance().loadIntoLabel(claim.itemImageUrl(), imgLabel, QSize(68, 68));
        itemRow->addWidget(imgLabel);
    }

    QVBoxLayout* itemDetails = new QVBoxLayout();
    itemDetails->setSpacing(4);

    QLabel* itemTitle = new QLabel(claim.itemTitle.isEmpty() ? QString("Item #%1").arg(claim.itemId) : claim.itemTitle, card);
    itemTitle->setStyleSheet("font-size: 16px; font-weight: 800; color: #0F172A;");
    itemDetails->addWidget(itemTitle);

    QLabel* itemLoc = new QLabel(QString("📍 Found Location: <b>%1</b>").arg(claim.itemLocation), card);
    itemLoc->setStyleSheet("font-size: 13px; color: #475569;");
    itemDetails->addWidget(itemLoc);

    itemRow->addLayout(itemDetails, 1);
    cardLayout->addLayout(itemRow);

    // 3. User's Submitted Verification Summary
    QFrame* answersBox = new QFrame(card);
    answersBox->setStyleSheet("background-color: #F8FAFC; border: 1px solid #F1F5F9; border-radius: 10px; padding: 10px;");
    QVBoxLayout* answersLayout = new QVBoxLayout(answersBox);
    answersLayout->setSpacing(6);

    QLabel* q1 = new QLabel(QString("<b>Where last seen:</b> %1").arg(claim.lastSeenLocation), answersBox);
    q1->setStyleSheet("font-size: 12px; color: #334155;");
    answersLayout->addWidget(q1);

    QLabel* q2 = new QLabel(QString("<b>Unique features provided:</b> %1").arg(claim.uniqueIdentifyingDetails), answersBox);
    q2->setStyleSheet("font-size: 12px; color: #334155;");
    answersLayout->addWidget(q2);

    QLabel* q3 = new QLabel(QString("<b>Contents:</b> %1").arg(claim.itemContents), answersBox);
    q3->setStyleSheet("font-size: 12px; color: #334155;");
    answersLayout->addWidget(q3);

    cardLayout->addWidget(answersBox);

    // 4. Conditional Status Blocks:
    if (claim.isPending()) {
        // Timeline & Pending Notice
        QFrame* timelineBox = new QFrame(card);
        timelineBox->setStyleSheet("background-color: #FFFBEB; border: 1px solid #FDE68A; border-radius: 10px; padding: 12px;");
        QVBoxLayout* tlLayout = new QVBoxLayout(timelineBox);
        tlLayout->setSpacing(10);

        QLabel* tlTitle = new QLabel("⏳ Claim Verification In Progress", timelineBox);
        tlTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #92400E;");
        tlLayout->addWidget(tlTitle);

        // Visual 3-step timeline
        QHBoxLayout* stepsLayout = new QHBoxLayout();
        stepsLayout->setSpacing(8);

        auto makeStep = [](const QString& num, const QString& label, bool active, bool current) {
            QFrame* f = new QFrame();
            QHBoxLayout* h = new QHBoxLayout(f);
            h->setContentsMargins(8, 6, 8, 6);
            h->setSpacing(6);
            QLabel* n = new QLabel(num);
            n->setStyleSheet(QString(
                "background-color: %1; color: %2; font-weight: bold; border-radius: 10px; font-size: 11px; padding: 2px 7px;"
            ).arg(active ? "#10B981" : (current ? "#F59E0B" : "#E2E8F0"), active || current ? "#FFFFFF" : "#64748B"));
            QLabel* l = new QLabel(label);
            l->setStyleSheet(QString("font-size: 12px; font-weight: %1; color: %2;").arg(active || current ? "bold" : "normal", active || current ? "#1E293B" : "#94A3B8"));
            h->addWidget(n);
            h->addWidget(l);
            return f;
        };

        stepsLayout->addWidget(makeStep("✓", "1. Submitted", true, false));
        QLabel* arr1 = new QLabel("➔");
        arr1->setStyleSheet("color: #CBD5E1; font-weight: bold;");
        stepsLayout->addWidget(arr1);

        stepsLayout->addWidget(makeStep("2", "Admin Reviewing", false, true));
        QLabel* arr2 = new QLabel("➔");
        arr2->setStyleSheet("color: #CBD5E1; font-weight: bold;");
        stepsLayout->addWidget(arr2);

        stepsLayout->addWidget(makeStep("3", "Ready for Pickup", false, false));
        stepsLayout->addStretch();
        tlLayout->addLayout(stepsLayout);

        QLabel* note = new QLabel(
            "Campus security administration is verifying your answers against the held physical item. You will receive an approval update once verified.",
            timelineBox
        );
        note->setStyleSheet("font-size: 11px; color: #78350F;");
        note->setWordWrap(true);
        tlLayout->addWidget(note);

        cardLayout->addWidget(timelineBox);
    }
    else if (claim.isApproved()) {
        // High-visibility Official Pickup Pass
        QFrame* passBox = new QFrame(card);
        passBox->setStyleSheet(
            "QFrame {"
            "   background-color: #ECFDF5;"
            "   border: 1.5px dashed #059669;"
            "   border-radius: 12px;"
            "   padding: 14px;"
            "}"
        );
        QVBoxLayout* passLayout = new QVBoxLayout(passBox);
        passLayout->setSpacing(10);

        QHBoxLayout* passHeader = new QHBoxLayout();
        QLabel* passTitle = new QLabel("🎫 OFFICIAL CAMPUS PICKUP PASS", passBox);
        passTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #065F46; letter-spacing: 0.5px;");
        passHeader->addWidget(passTitle);
        passHeader->addStretch();
        QLabel* passState = new QLabel("STATUS: READY FOR PICKUP", passBox);
        passState->setStyleSheet("background-color: #059669; color: #FFFFFF; font-size: 11px; font-weight: 800; padding: 4px 10px; border-radius: 6px;");
        passHeader->addWidget(passState);
        passLayout->addLayout(passHeader);

        QFrame* passLine = new QFrame(passBox);
        passLine->setFixedHeight(1);
        passLine->setStyleSheet("background-color: #A7F3D0;");
        passLayout->addWidget(passLine);

        QGridLayout* infoGrid = new QGridLayout();
        infoGrid->setHorizontalSpacing(16);
        infoGrid->setVerticalSpacing(8);

        infoGrid->addWidget(new QLabel("<b>Pickup Location:</b>", passBox), 0, 0);
        QLabel* locVal = new QLabel(claim.pickupLocation.isEmpty() ? "Admin Block - Room 102" : claim.pickupLocation, passBox);
        locVal->setStyleSheet("color: #065F46; font-weight: bold; font-size: 13px;");
        infoGrid->addWidget(locVal, 0, 1);

        infoGrid->addWidget(new QLabel("<b>Office Hours:</b>", passBox), 1, 0);
        QLabel* hrsVal = new QLabel("Monday – Friday: 09:00 AM – 04:00 PM", passBox);
        hrsVal->setStyleSheet("color: #047857; font-size: 12px;");
        infoGrid->addWidget(hrsVal, 1, 1);

        infoGrid->addWidget(new QLabel("<b>Required to Bring:</b>", passBox), 2, 0);
        QLabel* reqVal = new QLabel("🪪 Valid University Student ID Card + This Reference: <b>" + claim.claimReference + "</b>", passBox);
        reqVal->setStyleSheet("color: #047857; font-size: 12px;");
        infoGrid->addWidget(reqVal, 2, 1);

        passLayout->addLayout(infoGrid);

        QLabel* instr = new QLabel(
            "Instructions: Please visit the designated Admin Block counter in person. The security officer will inspect your ID card and reference code before physically handing over the item to you.",
            passBox
        );
        instr->setStyleSheet("font-size: 11px; color: #064E3B; font-style: italic;");
        instr->setWordWrap(true);
        passLayout->addWidget(instr);

        cardLayout->addWidget(passBox);
    }
    else if (claim.isRejected()) {
        // Rejection explanation box
        QFrame* rejBox = new QFrame(card);
        rejBox->setStyleSheet("background-color: #FEF2F2; border: 1px solid #FECDD3; border-radius: 10px; padding: 12px;");
        QVBoxLayout* rejLayout = new QVBoxLayout(rejBox);
        rejLayout->setSpacing(6);

        QLabel* rejTitle = new QLabel("⚠️ Claim Not Approved", rejBox);
        rejTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #991B1B;");
        rejLayout->addWidget(rejTitle);

        QString reasonText = claim.rejectionReason.isEmpty() ? "Verification answers did not match the physical item." : claim.rejectionReason;
        QLabel* rejReason = new QLabel(QString("<b>Reason:</b> %1").arg(reasonText), rejBox);
        rejReason->setStyleSheet("font-size: 12px; color: #B91C1C;");
        rejReason->setWordWrap(true);
        rejLayout->addWidget(rejReason);

        QLabel* rejGuide = new QLabel(
            "If you believe this was in error, please visit the Campus Security office or submit a new claim with clearer identifying photos or serial numbers.",
            rejBox
        );
        rejGuide->setStyleSheet("font-size: 11px; color: #7F1D1D;");
        rejGuide->setWordWrap(true);
        rejLayout->addWidget(rejGuide);

        cardLayout->addWidget(rejBox);
    }
    else if (claim.isHandedOver()) {
        // Completed return record
        QFrame* compBox = new QFrame(card);
        compBox->setStyleSheet("background-color: #F0FDF4; border: 1px solid #BBF7D0; border-radius: 10px; padding: 12px;");
        QVBoxLayout* compLayout = new QVBoxLayout(compBox);
        compLayout->setSpacing(6);

        QLabel* compTitle = new QLabel("🎉 Handover Completed — Item Successfully Returned!", compBox);
        compTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #166534;");
        compLayout->addWidget(compTitle);

        QString handDate = claim.handedOverAt.isEmpty() ? "Handed over at campus desk" : QString("Handed over on %1").arg(claim.handedOverAt);
        QLabel* compDetails = new QLabel(QString("Official Return Record: %1. Permanent audit trail preserved.").arg(handDate), compBox);
        compDetails->setStyleSheet("font-size: 12px; color: #15803D;");
        compLayout->addWidget(compDetails);

        cardLayout->addWidget(compBox);
    }

    return card;
}

