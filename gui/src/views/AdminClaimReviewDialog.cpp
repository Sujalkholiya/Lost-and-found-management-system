#include "AdminClaimReviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QInputDialog>
#include <QMessageBox>
#include "../network/ImageLoader.h"
#include "../components/Theme.h"

AdminClaimReviewDialog::AdminClaimReviewDialog(const Claim& claim, ApiClient* apiClient, QWidget* parent)
    : QDialog(parent),
      m_claim(claim),
      m_apiClient(apiClient),
      m_approveBtn(nullptr),
      m_rejectBtn(nullptr),
      m_handoverBtn(nullptr),
      m_handoverConfirmCheck(nullptr),
      m_pickupLocationEdit(nullptr)
{
    setObjectName("AdminClaimReviewDialog");
    setWindowTitle(QString("Review Claim: %1 (%2)").arg(claim.claimReference, claim.itemTitle));
    setFixedSize(740, 720);
    setStyleSheet("#AdminClaimReviewDialog { background-color: #FFFFFF; font-family: 'Segoe UI', sans-serif; }");

    setupUi();
}

void AdminClaimReviewDialog::setupUi()
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: #FFFFFF;");

    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(16);

    // 1. Top Header: Claim Reference + Live Status Badge
    QHBoxLayout* header = new QHBoxLayout();
    header->setSpacing(12);

    QVBoxLayout* titleBox = new QVBoxLayout();
    titleBox->setSpacing(4);

    QLabel* refTitle = new QLabel(QString("Claim Verification: %1").arg(m_claim.claimReference), this);
    refTitle->setStyleSheet("font-size: 20px; font-weight: 700; color: " + Theme::Text::Primary + "; font-family: monospace;");
    titleBox->addWidget(refTitle);

    QLabel* subTitle = new QLabel("Campus Security Lost & Found Verification Console", this);
    subTitle->setStyleSheet("font-size: 12px; color: " + Theme::Text::Secondary + ";");
    titleBox->addWidget(subTitle);

    header->addLayout(titleBox, 1);

    QLabel* statusBadge = new QLabel(m_claim.statusDisplayName(), this);
    statusBadge->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 13px; font-weight: bold; padding: 6px 14px; border-radius: 8px; border: 1px solid %2;"
    ).arg(m_claim.statusBadgeBg(), m_claim.statusColor()));
    header->addWidget(statusBadge);

    layout->addLayout(header);

    // 2. Claimant Details Card
    QFrame* claimantCard = new QFrame(this);
    claimantCard->setStyleSheet("background-color: #F8FAFC; border: 1px solid " + Theme::Border::Default + "; border-radius: 12px; padding: 14px;");
    QGridLayout* claimantGrid = new QGridLayout(claimantCard);
    claimantGrid->setHorizontalSpacing(20);
    claimantGrid->setVerticalSpacing(6);

    claimantGrid->addWidget(new QLabel("<b>Claimant Name:</b>", claimantCard), 0, 0);
    QLabel* nameVal = new QLabel(m_claim.claimantName.isEmpty() ? QString("User #%1").arg(m_claim.userId) : m_claim.claimantName, claimantCard);
    nameVal->setStyleSheet("color: " + Theme::Text::Primary + "; font-weight: 700;");
    claimantGrid->addWidget(nameVal, 0, 1);

    claimantGrid->addWidget(new QLabel("<b>Email:</b>", claimantCard), 0, 2);
    QLabel* emailVal = new QLabel(m_claim.claimantEmail.isEmpty() ? "N/A" : m_claim.claimantEmail, claimantCard);
    emailVal->setStyleSheet("color: " + Theme::Primary::Main + "; font-weight: 600;");
    claimantGrid->addWidget(emailVal, 0, 3);

    claimantGrid->addWidget(new QLabel("<b>Submitted At:</b>", claimantCard), 1, 0);
    QLabel* dateVal = new QLabel(m_claim.createdAt, claimantCard);
    dateVal->setStyleSheet("color: " + Theme::Text::Secondary + ";");
    claimantGrid->addWidget(dateVal, 1, 1);

    claimantGrid->addWidget(new QLabel("<b>Target Item:</b>", claimantCard), 1, 2);
    QLabel* itemVal = new QLabel(QString("#%1: %2").arg(m_claim.itemId).arg(m_claim.itemTitle), claimantCard);
    itemVal->setStyleSheet("color: " + Theme::Text::Primary + "; font-weight: 600;");
    claimantGrid->addWidget(itemVal, 1, 3);

    layout->addWidget(claimantCard);

    // 3. Side-by-Side Comparison Header
    QLabel* compHeader = new QLabel("Side-by-Side Verification Analysis", this);
    compHeader->setStyleSheet("font-size: 15px; font-weight: 700; color: " + Theme::Text::Primary + "; margin-top: 6px;");
    layout->addWidget(compHeader);

    QHBoxLayout* comparisonLayout = new QHBoxLayout();
    comparisonLayout->setSpacing(16);

    // Column 1: Item Record
    QFrame* itemBox = new QFrame(this);
    itemBox->setStyleSheet("background-color: #F8FAFC; border: 1px solid " + Theme::Border::Default + "; border-radius: 14px; padding: 16px;");
    QVBoxLayout* itemLayout = new QVBoxLayout(itemBox);
    itemLayout->setSpacing(10);

    QLabel* itemHeader = new QLabel("📦 Reported Item Record", itemBox);
    itemHeader->setStyleSheet("font-size: 14px; font-weight: 700; color: " + Theme::Text::Primary + ";");
    itemLayout->addWidget(itemHeader);

    if (m_claim.hasImage()) {
        QLabel* imgLabel = new QLabel(itemBox);
        imgLabel->setFixedHeight(140);
        imgLabel->setAlignment(Qt::AlignCenter);
        imgLabel->setStyleSheet("background-color: #FFFFFF; border: 1px solid " + Theme::Border::Default + "; border-radius: 8px;");
        ImageLoader::instance().loadIntoLabel(m_claim.itemImageUrl(), imgLabel, QSize(300, 140));
        itemLayout->addWidget(imgLabel);
    }

    auto addFieldBox = [](QVBoxLayout* l, const QString& label, const QString& val) {
        QLabel* lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 11px; font-weight: 700; color: " + Theme::Text::Secondary + "; text-transform: uppercase; letter-spacing: 0.5px;");
        QLabel* v = new QLabel(val.isEmpty() ? "None recorded" : val);
        v->setStyleSheet("font-size: 13px; color: " + Theme::Text::Primary + "; line-height: 130%;");
        v->setWordWrap(true);
        l->addWidget(lbl);
        l->addWidget(v);
    };

    addFieldBox(itemLayout, "Title", m_claim.itemTitle);
    addFieldBox(itemLayout, "Found Location", m_claim.itemLocation);
    addFieldBox(itemLayout, "Description / Notes", m_claim.itemDescription);
    addFieldBox(itemLayout, "Current Item Status", m_claim.itemStatus);
    itemLayout->addStretch();

    comparisonLayout->addWidget(itemBox, 1);

    // Column 2: Claimant's Answers
    QFrame* answersBox = new QFrame(this);
    answersBox->setStyleSheet("background-color: #EFF6FF; border: 1.5px solid #BFDBFE; border-radius: 14px; padding: 16px;");
    QVBoxLayout* answersLayout = new QVBoxLayout(answersBox);
    answersLayout->setSpacing(10);

    QLabel* answersHeader = new QLabel("👤 Claimant's Verification Answers", answersBox);
    answersHeader->setStyleSheet("font-size: 14px; font-weight: 700; color: #1D4ED8;");
    answersLayout->addWidget(answersHeader);

    addFieldBox(answersLayout, "Where Last Seen / Lost", m_claim.lastSeenLocation);
    addFieldBox(answersLayout, "Unique Identifying Features", m_claim.uniqueIdentifyingDetails);
    addFieldBox(answersLayout, "Contents / Inner Details", m_claim.itemContents);
    if (!m_claim.additionalDetails.isEmpty()) {
        addFieldBox(answersLayout, "Additional Remarks / Proof", m_claim.additionalDetails);
    }
    answersLayout->addStretch();

    comparisonLayout->addWidget(answersBox, 1);
    layout->addLayout(comparisonLayout);

    // 4. Action & Handover Section
    QFrame* actionBox = new QFrame(this);
    actionBox->setStyleSheet("background-color: #F8FAFC; border: 1px solid " + Theme::Border::Default + "; border-radius: 14px; padding: 18px; margin-top: 8px;");
    QVBoxLayout* actionLayout = new QVBoxLayout(actionBox);
    actionLayout->setSpacing(12);

    if (m_claim.isPending()) {
        QLabel* actTitle = new QLabel("Admin Review Decision", actionBox);
        actTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: " + Theme::Text::Primary + ";");
        actionLayout->addWidget(actTitle);

        QHBoxLayout* locRow = new QHBoxLayout();
        QLabel* locPrompt = new QLabel("Pickup Location:", actionBox);
        locPrompt->setStyleSheet("color: " + Theme::Text::Secondary + "; font-size: 13px; font-weight: 600;");
        locRow->addWidget(locPrompt);
        m_pickupLocationEdit = new QLineEdit(m_claim.pickupLocation.isEmpty() ? "Admin Block - Room 102" : m_claim.pickupLocation, actionBox);
        m_pickupLocationEdit->setStyleSheet("background: white; border: 1px solid " + Theme::Border::Default + "; border-radius: 8px; padding: 8px 12px; font-size: 13px; color: " + Theme::Text::Primary + ";");
        locRow->addWidget(m_pickupLocationEdit, 1);
        actionLayout->addLayout(locRow);

        QHBoxLayout* btns = new QHBoxLayout();
        btns->setSpacing(12);

        m_rejectBtn = new QPushButton("✕ Reject Claim", actionBox);
        m_rejectBtn->setCursor(Qt::PointingHandCursor);
        m_rejectBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #FEF2F2;"
            "   color: " + Theme::Status::Error + ";"
            "   border: 1px solid #FECDD3;"
            "   font-size: 13px;"
            "   font-weight: 600;"
            "   padding: 10px 18px;"
            "   border-radius: 10px;"
            "}"
            "QPushButton:hover { background-color: #DC2626; color: white; }"
        );
        connect(m_rejectBtn, &QPushButton::clicked, this, &AdminClaimReviewDialog::onRejectClicked);
        btns->addWidget(m_rejectBtn);

        m_approveBtn = new QPushButton("✓ Approve Claim (Ready for Pickup)", actionBox);
        m_approveBtn->setCursor(Qt::PointingHandCursor);
        m_approveBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #10B981;"
            "   color: #FFFFFF;"
            "   font-size: 13px;"
            "   font-weight: 600;"
            "   padding: 10px 22px;"
            "   border-radius: 10px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: #059669; }"
        );
        connect(m_approveBtn, &QPushButton::clicked, this, &AdminClaimReviewDialog::onApproveClicked);
        btns->addWidget(m_approveBtn, 1);

        actionLayout->addLayout(btns);
    }
    else if (m_claim.isApproved()) {
        QLabel* actTitle = new QLabel("Physical Handover & Return Verification", actionBox);
        actTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #065F46;");
        actionLayout->addWidget(actTitle);

        QLabel* instr = new QLabel(
            "This claim was approved. When the student visits the counter, verify their Student ID card against this claim reference. Once handed over, check the box below to mark the item as returned in the university system.",
            actionBox
        );
        instr->setStyleSheet("font-size: 12px; color: " + Theme::Text::Secondary + ";");
        instr->setWordWrap(true);
        actionLayout->addWidget(instr);

        m_handoverConfirmCheck = new QCheckBox(
            "I confirm that the claimant presented valid student ID and the physical item was handed over in person.",
            actionBox
        );
        m_handoverConfirmCheck->setStyleSheet("font-size: 12px; font-weight: 600; color: " + Theme::Text::Primary + "; margin: 4px 0;");
        actionLayout->addWidget(m_handoverConfirmCheck);

        QHBoxLayout* handBtns = new QHBoxLayout();
        handBtns->setSpacing(12);

        m_rejectBtn = new QPushButton("✕ Reject (Failed ID Check)", actionBox);
        m_rejectBtn->setCursor(Qt::PointingHandCursor);
        m_rejectBtn->setStyleSheet(
            "QPushButton { background: #FEF2F2; color: " + Theme::Status::Error + "; border: 1px solid #FECDD3; padding: 10px 16px; border-radius: 10px; font-weight: 600; font-size: 13px; }"
            "QPushButton:hover { background: #DC2626; color: white; }"
        );
        connect(m_rejectBtn, &QPushButton::clicked, this, &AdminClaimReviewDialog::onRejectClicked);
        handBtns->addWidget(m_rejectBtn);

        m_handoverBtn = new QPushButton("🤝 Complete Physical Handover", actionBox);
        m_handoverBtn->setCursor(Qt::PointingHandCursor);
        m_handoverBtn->setEnabled(false);
        m_handoverBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Theme::Primary::Main + ";"
            "   color: #FFFFFF;"
            "   font-size: 13px;"
            "   font-weight: 600;"
            "   padding: 10px 22px;"
            "   border-radius: 10px;"
            "   border: none;"
            "}"
            "QPushButton:hover { background-color: " + Theme::Primary::Dark + "; }"
            "QPushButton:disabled { background-color: #CBD5E1; color: #94A3B8; }"
        );
        connect(m_handoverConfirmCheck, &QCheckBox::toggled, m_handoverBtn, &QPushButton::setEnabled);
        connect(m_handoverBtn, &QPushButton::clicked, this, &AdminClaimReviewDialog::onHandoverClicked);
        handBtns->addWidget(m_handoverBtn, 1);

        actionLayout->addLayout(handBtns);
    }
    else if (m_claim.isHandedOver()) {
        QLabel* compLabel = new QLabel(QString("🎉 Item was handed over and marked as Returned on %1.").arg(m_claim.handedOverAt), actionBox);
        compLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #166534;");
        actionLayout->addWidget(compLabel);
    }
    else if (m_claim.isRejected()) {
        QLabel* rejLabel = new QLabel(QString("⚠️ Claim Rejected. Reason: %1").arg(m_claim.rejectionReason), actionBox);
        rejLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #991B1B;");
        rejLabel->setWordWrap(true);
        actionLayout->addWidget(rejLabel);
    }

    layout->addWidget(actionBox);

    // Close Button
    QPushButton* closeBtn = new QPushButton("Close", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: " + Theme::Text::Secondary + ";"
        "   border: 1px solid " + Theme::Border::Default + ";"
        "   border-radius: 10px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 10px 20px;"
        "}"
        "QPushButton:hover { background-color: " + Theme::Primary::Subtle + "; color: " + Theme::Primary::Main + "; border-color: " + Theme::Primary::Border + "; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);
}

void AdminClaimReviewDialog::onApproveClicked()
{
    if (!m_apiClient) return;

    QString pickupLoc = m_pickupLocationEdit ? m_pickupLocationEdit->text().trimmed() : "Admin Block - Room 102";

    m_approveBtn->setEnabled(false);
    m_approveBtn->setText("Approving...");

    m_apiClient->approveClaim(
        m_claim.id,
        pickupLoc,
        [this](const QJsonObject& res) {
            Q_UNUSED(res);
            QMessageBox::information(
                this,
                "Claim Approved",
                "Claim approved successfully!\nThe claimant's status has been updated to 'Ready for Pickup' and their digital pickup pass is active."
            );
            emit claimUpdated();
            accept();
        },
        [this](int code, const QString& err) {
            Q_UNUSED(code);
            m_approveBtn->setEnabled(true);
            m_approveBtn->setText("✓ Approve Claim (Ready for Pickup)");
            QMessageBox::critical(this, "Approval Failed", err.isEmpty() ? "Failed to approve claim." : err);
        }
    );
}

void AdminClaimReviewDialog::onRejectClicked()
{
    if (!m_apiClient) return;

    bool ok = false;
    QString reason = QInputDialog::getText(
        this,
        "Reject Ownership Claim",
        "Enter the required reason for rejecting this claim:\n(This will be visible to the student)",
        QLineEdit::Normal,
        "Identifying features and description did not match the held physical item.",
        &ok
    );

    if (!ok || reason.trimmed().isEmpty()) {
        if (ok) {
            QMessageBox::warning(this, "Reason Required", "You must provide a rejection reason.");
        }
        return;
    }

    if (m_rejectBtn) {
        m_rejectBtn->setEnabled(false);
    }

    m_apiClient->rejectClaim(
        m_claim.id,
        reason.trimmed(),
        [this](const QJsonObject& res) {
            Q_UNUSED(res);
            QMessageBox::information(this, "Claim Rejected", "The claim has been rejected with the recorded reason.");
            emit claimUpdated();
            accept();
        },
        [this](int code, const QString& err) {
            Q_UNUSED(code);
            if (m_rejectBtn) m_rejectBtn->setEnabled(true);
            QMessageBox::critical(this, "Rejection Failed", err.isEmpty() ? "Failed to reject claim." : err);
        }
    );
}

void AdminClaimReviewDialog::onHandoverClicked()
{
    if (!m_apiClient) return;

    if (!m_handoverConfirmCheck || !m_handoverConfirmCheck->isChecked()) {
        QMessageBox::warning(this, "Confirmation Required", "Please check the ID verification confirmation box.");
        return;
    }

    m_handoverBtn->setEnabled(false);
    m_handoverBtn->setText("Processing Handover...");

    m_apiClient->handoverClaim(
        m_claim.id,
        [this](const QJsonObject& res) {
            Q_UNUSED(res);
            QMessageBox::information(
                this,
                "Item Handed Over Successfully",
                "Physical handover recorded!\nItem status in the university database is now updated to 'RETURNED'."
            );
            emit claimUpdated();
            accept();
        },
        [this](int code, const QString& err) {
            Q_UNUSED(code);
            m_handoverBtn->setEnabled(true);
            m_handoverBtn->setText("🤝 Complete Physical Handover");
            QMessageBox::critical(this, "Handover Failed", err.isEmpty() ? "Failed to complete handover." : err);
        }
    );
}

