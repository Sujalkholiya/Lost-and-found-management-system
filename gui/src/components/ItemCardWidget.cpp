#include "ItemCardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "../network/ImageLoader.h"
#include "../auth/AuthManager.h"

ItemCardWidget::ItemCardWidget(const Item &item, QWidget *parent)
    : QFrame(parent)
    , m_item(item)
    , m_typeBadge(new QLabel(this))
    , m_titleLabel(new QLabel(item.title, this))
    , m_descLabel(new QLabel(item.description, this))
    , m_locationLabel(new QLabel(QString("📍 %1").arg(item.location.isEmpty() ? "Campus General" : item.location), this))
    , m_userLabel(new QLabel(QString("👤 Student #%1").arg(item.userId), this))
    , m_imageThumbnail(new QLabel(this))
    , m_detailsBtn(new QPushButton("View Details →", this))
    , m_deleteBtn(new QPushButton("🗑 Delete", this))
{
    setObjectName("itemCard");
    setFrameShape(QFrame::StyledPanel);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    bool isLost = item.isLost();

    // 1. Type Badge Styling (matching reference design pill badges)
    m_typeBadge->setText(isLost ? "• LOST" : "• FOUND");
    m_typeBadge->setAlignment(Qt::AlignCenter);
    if (isLost) {
        m_typeBadge->setStyleSheet(
            "background-color: #FEF2F2;"
            "color: #EF4444;"
            "font-size: 10px;"
            "font-weight: 800;"
            "padding: 4px 10px;"
            "border-radius: 10px;"
            "border: 1px solid #FECDD3;"
        );
    } else {
        m_typeBadge->setStyleSheet(
            "background-color: #EAFBF4;"
            "color: #16A66A;"
            "font-size: 10px;"
            "font-weight: 800;"
            "padding: 4px 10px;"
            "border-radius: 10px;"
            "border: 1px solid #A7F3D0;"
        );
    }

    // 2. Typography
    m_titleLabel->setStyleSheet("font-size: 15px; font-weight: 800; color: #172B4D; background: transparent;");
    m_titleLabel->setWordWrap(true);

    m_descLabel->setStyleSheet("font-size: 12px; color: #6B7FA3; background: transparent; line-height: 130%;");
    m_descLabel->setWordWrap(true);
    m_descLabel->setMaximumHeight(36);

    m_locationLabel->setStyleSheet(
        "font-size: 11px;"
        "font-weight: 600;"
        "color: #6B7FA3;"
        "background-color: #F4F8FF;"
        "padding: 4px 10px;"
        "border-radius: 6px;"
        "border: 1px solid #E2EAF4;"
    );

    m_userLabel->setStyleSheet(
        "font-size: 11px;"
        "font-weight: 600;"
        "color: #172B4D;"
        "background-color: #F4F8FF;"
        "padding: 4px 10px;"
        "border-radius: 6px;"
        "border: 1px solid #E2EAF4;"
    );

    // 3. CTA Button
    m_detailsBtn->setCursor(Qt::PointingHandCursor);
    m_detailsBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #EFF6FF;"
        "   color: #2563EB;"
        "   border: 1px solid #BFDBFE;"
        "   border-radius: 8px;"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   padding: 6px 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   border: 1px solid #2563EB;"
        "}"
    );

    connect(m_detailsBtn, &QPushButton::clicked, this, [this]() {
        emit viewDetailsRequested(m_item);
    });

    // 4. Delete Button (visible ONLY if current user is owner or admin)
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FEF2F2;"
        "   color: #EF4444;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 8px;"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EF4444;"
        "   color: #FFFFFF;"
        "   border: 1px solid #DC2626;"
        "}"
    );

    bool canDelete = false;
    if (AuthManager::instance().isLoggedIn()) {
        int currentUserId = AuthManager::instance().userId();
        bool isAdmin = AuthManager::instance().isAdmin();
        if (isAdmin || (currentUserId > 0 && currentUserId == m_item.userId)) {
            canDelete = true;
        }
    }
    m_deleteBtn->setVisible(canDelete);

    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteItemRequested(m_item);
    });

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 14, 18, 14);
    mainLayout->setSpacing(8);

    QHBoxLayout *topHeader = new QHBoxLayout();
    topHeader->setSpacing(8);
    topHeader->addWidget(m_typeBadge);
    topHeader->addWidget(m_userLabel);
    topHeader->addStretch();
    if (!item.createdAt.isEmpty()) {
        QLabel *dateLabel = new QLabel("🕒 " + item.createdAt.left(10), this);
        dateLabel->setStyleSheet("font-size: 11px; font-weight: 500; color: #94A3B8; background: transparent;");
        topHeader->addWidget(dateLabel);
    }

    QHBoxLayout *footer = new QHBoxLayout();
    footer->setSpacing(8);
    footer->addWidget(m_locationLabel);
    footer->addStretch();
    footer->addWidget(m_deleteBtn);
    footer->addWidget(m_detailsBtn);

    mainLayout->addLayout(topHeader);

    // Body: image thumbnail (if available) + text content
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(12);

    m_imageThumbnail->setFixedSize(68, 68);
    m_imageThumbnail->setAlignment(Qt::AlignCenter);
    m_imageThumbnail->setStyleSheet(
        "border: 1px solid #E2E8F0;"
        "border-radius: 8px;"
        "background-color: #F8FAFC;"
    );

    if (item.hasImage()) {
        m_imageThumbnail->setText("🖼️");
        m_imageThumbnail->setStyleSheet(
            "border: 1px solid #CBD5E1;"
            "border-radius: 8px;"
            "background-color: #F1F5F9;"
            "font-size: 20px;"
        );
        ImageLoader::instance().loadIntoLabel(item.imageUrl(), m_imageThumbnail, QSize(68, 68));
    } else {
        m_imageThumbnail->setText(isLost ? "🔍" : "📦");
        m_imageThumbnail->setStyleSheet(
            "border: 1px solid #E2E8F0;"
            "border-radius: 8px;"
            "background-color: #F8FAFC;"
            "font-size: 24px;"
        );
    }
    bodyLayout->addWidget(m_imageThumbnail);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(4);
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_descLabel);
    bodyLayout->addLayout(textLayout, 1);

    mainLayout->addLayout(bodyLayout);
    mainLayout->addSpacing(2);
    mainLayout->addLayout(footer);

    setStyleSheet(
        "ItemCardWidget#itemCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2EAF4;"
        "   border-radius: 18px;"
        "}"
        "ItemCardWidget#itemCard:hover {"
        "   border: 1.5px solid #3B82F6;"
        "   background-color: #FFFFFF;"
        "}"
    );
}

void ItemCardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit viewDetailsRequested(m_item);
    }
    QFrame::mousePressEvent(event);
}

void ItemCardWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit viewDetailsRequested(m_item);
}