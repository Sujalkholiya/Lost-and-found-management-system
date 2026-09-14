#include "ItemDetailsDialog.h"
#include "ClaimFormDialog.h"
#include "../components/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include "../network/ImageLoader.h"
#include "../auth/AuthManager.h"

ItemDetailsDialog::ItemDetailsDialog(const Item& item, QWidget* parent)
    : ItemDetailsDialog(item, nullptr, parent)
{
}

ItemDetailsDialog::ItemDetailsDialog(const Item& item, ApiClient* apiClient, QWidget* parent)
    : QDialog(parent),
      m_item(item),
      m_apiClient(apiClient),
      m_hasClaim(false),
      m_claimBtn(nullptr),
      m_claimStatusBox(nullptr),
      m_claimStatusLabel(nullptr)
{
    setObjectName("ItemDetailsDialog");
    setWindowTitle(item.title);
    setFixedSize(560, 680);
    setStyleSheet("#ItemDetailsDialog { background-color: #FFFFFF; font-family: 'Segoe UI', sans-serif; }");

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

    // Top Header: Type Badge / Returned Badge
    QHBoxLayout* header = new QHBoxLayout();
    bool isReturned = (item.status.toLower() == "returned");

    QLabel* badge = new QLabel(this);
    if (isReturned) {
        badge->setText("✓ RETURNED TO OWNER");
        badge->setStyleSheet("background-color: #F0FDF4; color: #15803D; font-size: 12px; font-weight: bold; padding: 6px 14px; border-radius: 8px; border: 1px solid #BBF7D0;");
    } else if (item.isLost()) {
        badge->setText("🔴 LOST ITEM");
        badge->setStyleSheet("background-color: #FFF1F2; color: #E11D48; font-size: 12px; font-weight: bold; padding: 6px 14px; border-radius: 8px; border: 1px solid #FECDD3;");
    } else {
        badge->setText("🟢 FOUND ITEM — AVAILABLE TO CLAIM");
        badge->setStyleSheet("background-color: #ECFDF5; color: #059669; font-size: 12px; font-weight: bold; padding: 6px 14px; border-radius: 8px; border: 1px solid #A7F3D0;");
    }
    header->addWidget(badge);
    header->addStretch();
    layout->addLayout(header);

    // Title
    QLabel* titleLabel = new QLabel(item.title, this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: " + Theme::Text::Primary + ";");
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    // Attached Image Preview (if present)
    if (item.hasImage()) {
        QLabel* imageLabel = new QLabel(this);
        imageLabel->setFixedHeight(220);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #F8FAFC;"
            "   border: 1px solid " + Theme::Border::Default + ";"
            "   border-radius: 12px;"
            "}"
        );
        imageLabel->setText("🖼️ Loading item photo...");
        ImageLoader::instance().loadIntoLabel(item.imageUrl(), imageLabel, QSize(460, 220));
        layout->addWidget(imageLabel);
    }

    // Metadata Row
    QFrame* metaBox = new QFrame(this);
    metaBox->setStyleSheet("background-color: #F8FAFC; border: 1px solid " + Theme::Border::Default + "; border-radius: 12px; padding: 14px;");
    QVBoxLayout* metaLayout = new QVBoxLayout(metaBox);
    metaLayout->setSpacing(8);

    QLabel* locLabel = new QLabel(QString("📍 <b>Location:</b> %1").arg(item.location), metaBox);
    locLabel->setStyleSheet("font-size: 13px; color: " + Theme::Text::Primary + ";");
    metaLayout->addWidget(locLabel);

    QLabel* userLabel = new QLabel(QString("👤 <b>Reported by:</b> User #%1").arg(item.userId), metaBox);
    userLabel->setStyleSheet("font-size: 13px; color: " + Theme::Text::Primary + ";");
    metaLayout->addWidget(userLabel);

    if (!item.createdAt.isEmpty()) {
        QLabel* dateLabel = new QLabel(QString("🕒 <b>Date:</b> %1").arg(item.createdAt), metaBox);
        dateLabel->setStyleSheet("font-size: 13px; color: " + Theme::Text::Secondary + ";");
        metaLayout->addWidget(dateLabel);
    }
    layout->addWidget(metaBox);

    // Description Section
    QLabel* descTitle = new QLabel("Description", this);
    descTitle->setStyleSheet("font-size: 11px; font-weight: 700; color: " + Theme::Text::Secondary + "; text-transform: uppercase; letter-spacing: 0.5px;");
    layout->addWidget(descTitle);

    QLabel* descLabel = new QLabel(item.description, this);
    descLabel->setStyleSheet("font-size: 14px; color: " + Theme::Text::Primary + "; line-height: 140%;");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    // Campus Safety Guidelines box
    QFrame* guideBox = new QFrame(this);
    guideBox->setStyleSheet("background-color: #EFF6FF; border: 1px solid #BFDBFE; border-radius: 10px; padding: 12px;");
    QVBoxLayout* guideLayout = new QVBoxLayout(guideBox);
    QLabel* guideTitle = new QLabel("🛡️ Campus Recovery Guideline", guideBox);
    guideTitle->setStyleSheet("font-size: 12px; font-weight: 700; color: #1D4ED8;");
    QLabel* guideText = new QLabel("All item handovers are coordinated through the Admin Block campus security desk after identity verification.", guideBox);
    guideText->setStyleSheet("font-size: 12px; color: #2563EB;");
    guideText->setWordWrap(true);
    guideLayout->addWidget(guideTitle);
    guideLayout->addWidget(guideText);
    layout->addWidget(guideBox);

    // Dynamic Claim Status Box
    m_claimStatusBox = new QFrame(this);
    m_claimStatusBox->setStyleSheet("border-radius: 10px; padding: 12px;");
    QVBoxLayout* csLayout = new QVBoxLayout(m_claimStatusBox);
    m_claimStatusLabel = new QLabel(m_claimStatusBox);
    m_claimStatusLabel->setWordWrap(true);
    m_claimStatusLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
    csLayout->addWidget(m_claimStatusLabel);
    m_claimStatusBox->hide();
    layout->addWidget(m_claimStatusBox);

    layout->addStretch();

    // Buttons Row: Claim Item / Delete / Close
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    int currentUserId = AuthManager::instance().userId();
    bool isAdmin = AuthManager::instance().isAdmin();
    bool canDelete = (isAdmin || (currentUserId > 0 && currentUserId == m_item.userId));

    if (canDelete && m_apiClient) {
        QPushButton* deleteBtn = new QPushButton("🗑 Delete Item", this);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #FEF2F2;"
            "   color: #DC2626;"
            "   border: 1px solid #FECDD3;"
            "   border-radius: 10px;"
            "   font-weight: 600;"
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

    // Claim Button for FOUND items
    if (m_item.isFound()) {
        m_claimBtn = new QPushButton(this);
        m_claimBtn->setCursor(Qt::PointingHandCursor);

        if (isReturned) {
            m_claimBtn->setText("✓ Item Returned to Owner");
            m_claimBtn->setEnabled(false);
            m_claimBtn->setStyleSheet("background-color: #E2E8F0; color: #64748B; border: none; border-radius: 10px; padding: 10px 18px; font-weight: 600;");
        } else if (!AuthManager::instance().isLoggedIn()) {
            m_claimBtn->setText("🔒 Sign In to Claim");
            m_claimBtn->setStyleSheet("background-color: #FFFFFF; color: " + Theme::Text::Secondary + "; border: 1px solid " + Theme::Border::Default + "; border-radius: 10px; padding: 10px 18px; font-weight: 600;");
            connect(m_claimBtn, &QPushButton::clicked, this, [this]() {
                QMessageBox::information(this, "Sign In Required", "Please sign in to your student account to claim this item.");
            });
        } else if (currentUserId == m_item.userId) {
            m_claimBtn->setText("You Reported This Item");
            m_claimBtn->setEnabled(false);
            m_claimBtn->setStyleSheet("background-color: #F1F5F9; color: #64748B; border: 1px solid #E2E8F0; border-radius: 10px; padding: 10px 18px; font-weight: 600;");
        } else {
            m_claimBtn->setText("🎯 Claim This Item");
            m_claimBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: " + Theme::Primary::Main + ";"
                "   color: #FFFFFF;"
                "   font-weight: 600;"
                "   font-size: 13px;"
                "   padding: 10px 20px;"
                "   border-radius: 10px;"
                "   border: none;"
                "}"
                "QPushButton:hover { background-color: " + Theme::Primary::Dark + "; }"
                "QPushButton:disabled { background-color: #CBD5E1; color: #64748B; }"
            );
            connect(m_claimBtn, &QPushButton::clicked, this, &ItemDetailsDialog::onClaimClicked);
        }

        btnLayout->addWidget(m_claimBtn, 1);
    }

    QPushButton* closeBtn = new QPushButton("Close", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: " + Theme::Text::Secondary + ";"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "   padding: 10px 24px;"
        "   border-radius: 10px;"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "}"
        "QPushButton:hover { background-color: " + Theme::Primary::Subtle + "; color: " + Theme::Primary::Main + "; border-color: " + Theme::Primary::Border + "; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);

    layout->addLayout(btnLayout);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);

    if (m_item.isFound() && !isReturned && AuthManager::instance().isLoggedIn() && currentUserId != m_item.userId) {
        checkClaimStatus();
    }
}

void ItemDetailsDialog::checkClaimStatus()
{
    if (!m_apiClient) return;

    m_apiClient->getMyClaimForItem(
        m_item.id,
        [this](bool hasClaim, const Claim& claim) {
            m_hasClaim = hasClaim;
            m_currentClaim = claim;

            if (hasClaim && m_claimBtn) {
                m_claimStatusBox->show();

                if (claim.isPending()) {
                    m_claimStatusBox->setStyleSheet("background-color: #FFFBEB; border: 1px solid #FDE68A;");
                    m_claimStatusLabel->setStyleSheet("color: #92400E; font-size: 12px;");
                    m_claimStatusLabel->setText(
                        QString("⏳ You have a pending claim for this item (Ref: %1). Under administrative review.")
                            .arg(claim.claimReference)
                    );
                    m_claimBtn->setText("Claim Pending Review");
                    m_claimBtn->setEnabled(false);
                } else if (claim.isApproved()) {
                    m_claimStatusBox->setStyleSheet("background-color: #ECFDF5; border: 1.5px solid #059669;");
                    m_claimStatusLabel->setStyleSheet("color: #065F46; font-size: 12px;");
                    m_claimStatusLabel->setText(
                        QString("🎉 Your claim (%1) is APPROVED! Please visit %2 with your student ID.")
                            .arg(claim.claimReference, claim.pickupLocation)
                    );
                    m_claimBtn->setText("✓ Approved — Ready for Pickup");
                    m_claimBtn->setEnabled(false);
                } else if (claim.isHandedOver()) {
                    m_claimStatusBox->setStyleSheet("background-color: #F0FDF4; border: 1px solid #BBF7D0;");
                    m_claimStatusLabel->setStyleSheet("color: #15803D; font-size: 12px;");
                    m_claimStatusLabel->setText("🎉 Item handed over and officially returned to you.");
                    m_claimBtn->setText("✓ Item Handed Over");
                    m_claimBtn->setEnabled(false);
                } else if (claim.isRejected()) {
                    m_claimStatusBox->setStyleSheet("background-color: #FEF2F2; border: 1px solid #FECDD3;");
                    m_claimStatusLabel->setStyleSheet("color: #991B1B; font-size: 12px;");
                    m_claimStatusLabel->setText(
                        QString("⚠️ Previous claim not approved (%1). You may submit a new claim with clearer details.")
                            .arg(claim.rejectionReason)
                    );
                    m_claimBtn->setText("🎯 Resubmit Claim");
                    m_claimBtn->setEnabled(true);
                }
            }
        },
        [this](int code, const QString& err) {
            Q_UNUSED(code);
            Q_UNUSED(err);
        }
    );
}

void ItemDetailsDialog::onClaimClicked()
{
    ClaimFormDialog dlg(m_item, m_apiClient, this);
    connect(&dlg, &ClaimFormDialog::claimSubmitted, this, [this](const Claim& claim) {
        emit claimSubmitted(m_item.id);
        checkClaimStatus();
    });
    dlg.exec();
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
