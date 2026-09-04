#include "ItemDetailsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include "../network/ImageLoader.h"
#include "../auth/AuthManager.h"

ItemDetailsDialog::ItemDetailsDialog(const Item& item, ApiClient* apiClient, QWidget* parent)
    : QDialog(parent),
      m_item(item),
      m_apiClient(apiClient)
{
    setWindowTitle(item.title);
    setFixedSize(540, 640);
    setStyleSheet("background-color: #FFFFFF;");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: #FFFFFF;");

    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(16);

    // Top Header: Type Badge + Close button
    QHBoxLayout* header = new QHBoxLayout();
    QLabel* badge = new QLabel(item.isLost() ? "🔴 LOST ITEM" : "🟢 FOUND ITEM", this);
    badge->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 12px; font-weight: bold; padding: 6px 14px; border-radius: 8px; border: 1px solid %3;"
    ).arg(
        item.isLost() ? "#FFF1F2" : "#ECFDF5",
        item.isLost() ? "#E11D48" : "#059669",
        item.isLost() ? "#FECDD3" : "#A7F3D0"
    ));
    header->addWidget(badge);
    header->addStretch();
    layout->addLayout(header);

    // Title
    QLabel* titleLabel = new QLabel(item.title, this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 800; color: #0F172A;");
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    // Attached Image Preview (if present)
    if (item.hasImage()) {
        QLabel* imageLabel = new QLabel(this);
        imageLabel->setFixedHeight(220);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #F1F5F9;"
            "   border: 1px solid #CBD5E1;"
            "   border-radius: 12px;"
            "}"
        );
        imageLabel->setText("🖼️ Loading item photo...");
        ImageLoader::instance().loadIntoLabel(item.imageUrl(), imageLabel, QSize(460, 220));
        layout->addWidget(imageLabel);
    }

    // Metadata Row
    QFrame* metaBox = new QFrame(this);
    metaBox->setStyleSheet("background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 12px; padding: 12px;");
    QVBoxLayout* metaLayout = new QVBoxLayout(metaBox);
    metaLayout->setSpacing(8);

    QLabel* locLabel = new QLabel(QString("📍 <b>Location:</b> %1").arg(item.location), metaBox);
    locLabel->setStyleSheet("font-size: 13px; color: #334155;");
    metaLayout->addWidget(locLabel);

    QLabel* userLabel = new QLabel(QString("👤 <b>Reported by:</b> User #%1").arg(item.userId), metaBox);
    userLabel->setStyleSheet("font-size: 13px; color: #334155;");
    metaLayout->addWidget(userLabel);

    if (!item.createdAt.isEmpty()) {
        QLabel* dateLabel = new QLabel(QString("🕒 <b>Date:</b> %1").arg(item.createdAt), metaBox);
        dateLabel->setStyleSheet("font-size: 13px; color: #334155;");
        metaLayout->addWidget(dateLabel);
    }
    layout->addWidget(metaBox);

    // Description Section
    QLabel* descTitle = new QLabel("Description", this);
    descTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #64748B; text-transform: uppercase;");
    layout->addWidget(descTitle);

    QLabel* descLabel = new QLabel(item.description, this);
    descLabel->setStyleSheet("font-size: 14px; color: #1E293B; line-height: 140%;");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    // Campus Safety Guidelines box
    QFrame* guideBox = new QFrame(this);
    guideBox->setStyleSheet("background-color: #EEF2FF; border: 1px solid #C7D2FE; border-radius: 10px; padding: 10px;");
    QVBoxLayout* guideLayout = new QVBoxLayout(guideBox);
    QLabel* guideTitle = new QLabel("🛡️ Campus Recovery Guideline", guideBox);
    guideTitle->setStyleSheet("font-size: 12px; font-weight: bold; color: #3730A3;");
    QLabel* guideText = new QLabel("Always arrange item handovers in well-lit, public campus areas like the Student Center or Library front desk.", guideBox);
    guideText->setStyleSheet("font-size: 11px; color: #4338CA;");
    guideText->setWordWrap(true);
    guideLayout->addWidget(guideTitle);
    guideLayout->addWidget(guideText);
    layout->addWidget(guideBox);

    layout->addStretch();

    // Close Button
    // Buttons Layout: Delete (if authorized) + Done
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    bool canDelete = false;
    if (AuthManager::instance().isLoggedIn()) {
        int currentUserId = AuthManager::instance().userId();
        bool isAdmin = AuthManager::instance().isAdmin();
        if (isAdmin || (currentUserId > 0 && currentUserId == m_item.userId)) {
            canDelete = true;
        }
    }

    if (canDelete && m_apiClient) {
        QPushButton* deleteBtn = new QPushButton("🗑 Delete Item", this);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #FEF2F2;"
            "   color: #DC2626;"
            "   border: 1px solid #FECDD3;"
            "   border-radius: 10px;"
            "   font-weight: bold;"
            "   font-size: 13px;"
            "   padding: 10px 18px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #DC2626;"
            "   color: #FFFFFF;"
            "   border: 1px solid #B91C1C;"
            "}"
        );
        connect(deleteBtn, &QPushButton::clicked, this, &ItemDetailsDialog::onDeleteClicked);
        btnLayout->addWidget(deleteBtn);
    }

    QPushButton* closeBtn = new QPushButton("Done", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "   padding: 10px 24px;"
        "   border-radius: 10px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn, 1);

    layout->addLayout(btnLayout);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);
}

void ItemDetailsDialog::onDeleteClicked()
{
    if (!m_apiClient) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Item Report",
        QString("Are you sure you want to delete \"%1\"?\nThis cannot be undone.").arg(m_item.title),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    m_apiClient->deleteItem(
        m_item.id,
        [this](const QJsonObject &response) {
            Q_UNUSED(response);
            emit itemDeleted(m_item.id);
            accept();
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

