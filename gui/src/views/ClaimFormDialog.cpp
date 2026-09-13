#include "ClaimFormDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include "../network/ImageLoader.h"
#include "../auth/AuthManager.h"

ClaimFormDialog::ClaimFormDialog(const Item& item, ApiClient* apiClient, QWidget* parent)
    : QDialog(parent),
      m_item(item),
      m_apiClient(apiClient)
{
    setObjectName("ClaimFormDialog");
    setWindowTitle(QString("Claim Item: %1").arg(item.title));
    setFixedSize(580, 720);
    setStyleSheet("#ClaimFormDialog { background-color: #FFFFFF; font-family: 'Segoe UI', sans-serif; }");

    setupUi();
}

void ClaimFormDialog::setupUi()
{
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

    // Header Title
    QLabel* titleLabel = new QLabel("📋 Item Ownership Verification", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 800; color: #0F172A;");
    layout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel(
        "To protect campus property and prevent unauthorized claims, please provide specific verification answers that only the genuine owner would know.",
        this
    );
    subtitleLabel->setStyleSheet("font-size: 13px; color: #64748B; line-height: 140%;");
    subtitleLabel->setWordWrap(true);
    layout->addWidget(subtitleLabel);

    // Item Summary Card
    QFrame* itemCard = new QFrame(this);
    itemCard->setStyleSheet(
        "QFrame {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 12px;"
        "}"
    );
    QHBoxLayout* itemCardLayout = new QHBoxLayout(itemCard);
    itemCardLayout->setSpacing(12);

    if (m_item.hasImage()) {
        QLabel* imgLabel = new QLabel(itemCard);
        imgLabel->setFixedSize(72, 72);
        imgLabel->setStyleSheet("background-color: #E2E8F0; border-radius: 8px; border: 1px solid #CBD5E1;");
        imgLabel->setAlignment(Qt::AlignCenter);
        ImageLoader::instance().loadIntoLabel(m_item.imageUrl(), imgLabel, QSize(72, 72));
        itemCardLayout->addWidget(imgLabel);
    }

    QVBoxLayout* itemInfoLayout = new QVBoxLayout();
    itemInfoLayout->setSpacing(4);

    QLabel* itemTitle = new QLabel(m_item.title, itemCard);
    itemTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #0F172A;");
    itemInfoLayout->addWidget(itemTitle);

    QLabel* itemMeta = new QLabel(QString("📍 %1  •  Found Item #%2").arg(m_item.location).arg(m_item.id), itemCard);
    itemMeta->setStyleSheet("font-size: 12px; color: #64748B;");
    itemInfoLayout->addWidget(itemMeta);

    QLabel* itemBadge = new QLabel("Verification Review by Campus Admin Required", itemCard);
    itemBadge->setStyleSheet("font-size: 11px; font-weight: 600; color: #0284C7;");
    itemInfoLayout->addWidget(itemBadge);

    itemCardLayout->addLayout(itemInfoLayout, 1);
    layout->addWidget(itemCard);

    // Form inputs helper lambda
    auto createFieldLabel = [this](const QString& text, bool required = true) {
        QLabel* label = new QLabel(required ? QString("%1 <font color='#EF4444'>*</font>").arg(text) : text, this);
        label->setStyleSheet("font-size: 13px; font-weight: 700; color: #1E293B; margin-top: 4px;");
        return label;
    };

    const QString inputStyle =
        "QLineEdit, QTextEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 12px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus, QTextEdit:focus {"
        "   border: 1.5px solid #2563EB;"
        "   background-color: #FFFFFF;"
        "}";

    // Question 1: Where last seen
    layout->addWidget(createFieldLabel("1. Where did you last have or misplace this item?"));
    m_lastSeenEdit = new QLineEdit(this);
    m_lastSeenEdit->setPlaceholderText("e.g. Central Library 2nd floor, Cafeteria booth, Computer Lab 3...");
    m_lastSeenEdit->setStyleSheet(inputStyle);
    layout->addWidget(m_lastSeenEdit);

    // Question 2: Unique features
    layout->addWidget(createFieldLabel("2. Unique identifying features"));
    m_uniqueDetailsEdit = new QTextEdit(this);
    m_uniqueDetailsEdit->setPlaceholderText("e.g. Specific scratches, stickers on back, color of phone cover, lock screen wallpaper, brand engravings...");
    m_uniqueDetailsEdit->setFixedHeight(68);
    m_uniqueDetailsEdit->setStyleSheet(inputStyle);
    layout->addWidget(m_uniqueDetailsEdit);

    // Question 3: Contents / Attachments
    layout->addWidget(createFieldLabel("3. Contents, attachments, or inner details"));
    m_contentsEdit = new QTextEdit(this);
    m_contentsEdit->setPlaceholderText("e.g. What is inside the bag or wallet? Specific cards, files, keychains, coins, brand names...");
    m_contentsEdit->setFixedHeight(68);
    m_contentsEdit->setStyleSheet(inputStyle);
    layout->addWidget(m_contentsEdit);

    // Question 4: Additional info
    layout->addWidget(createFieldLabel("4. Additional proof or remarks", false));
    m_additionalEdit = new QLineEdit(this);
    m_additionalEdit->setPlaceholderText("e.g. Approximate time lost, serial number, purchase receipt note (optional)...");
    m_additionalEdit->setStyleSheet(inputStyle);
    layout->addWidget(m_additionalEdit);

    // Affirmation Checkbox
    m_affirmationCheckbox = new QCheckBox(
        "I declare under the University Student Code of Conduct that this item is my personal property and all details provided are true.",
        this
    );
    m_affirmationCheckbox->setStyleSheet(
        "QCheckBox {"
        "   font-size: 12px;"
        "   color: #334155;"
        "   spacing: 8px;"
        "   margin-top: 8px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 18px;"
        "   height: 18px;"
        "   border-radius: 4px;"
        "   border: 1px solid #94A3B8;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color: #2563EB;"
        "   border-color: #2563EB;"
        "}"
    );
    layout->addWidget(m_affirmationCheckbox);

    // Error label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("font-size: 12px; color: #DC2626; font-weight: 600; padding: 4px 0;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    layout->addWidget(m_errorLabel);

    // Action buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 10px 20px;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; }"
    );
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    m_submitBtn = new QPushButton("Submit Claim Request", this);
    m_submitBtn->setCursor(Qt::PointingHandCursor);
    m_submitBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 8px;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 10px 24px;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
        "QPushButton:disabled { background-color: #94A3B8; color: #E2E8F0; }"
    );
    connect(m_submitBtn, &QPushButton::clicked, this, &ClaimFormDialog::onSubmitClicked);
    btnLayout->addWidget(m_submitBtn, 1);

    layout->addLayout(btnLayout);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);

    // Connect signals for validation
    connect(m_lastSeenEdit, &QLineEdit::textChanged, this, &ClaimFormDialog::validateForm);
    connect(m_uniqueDetailsEdit, &QTextEdit::textChanged, this, &ClaimFormDialog::validateForm);
    connect(m_contentsEdit, &QTextEdit::textChanged, this, &ClaimFormDialog::validateForm);
    connect(m_affirmationCheckbox, &QCheckBox::toggled, this, &ClaimFormDialog::validateForm);

    validateForm();
}

void ClaimFormDialog::validateForm()
{
    bool valid = !m_lastSeenEdit->text().trimmed().isEmpty() &&
                 !m_uniqueDetailsEdit->toPlainText().trimmed().isEmpty() &&
                 !m_contentsEdit->toPlainText().trimmed().isEmpty() &&
                 m_affirmationCheckbox->isChecked();

    m_submitBtn->setEnabled(valid);
    if (m_errorLabel->isVisible()) {
        m_errorLabel->hide();
    }
}

void ClaimFormDialog::onSubmitClicked()
{
    QString lastSeen = m_lastSeenEdit->text().trimmed();
    QString uniqueDetails = m_uniqueDetailsEdit->toPlainText().trimmed();
    QString contents = m_contentsEdit->toPlainText().trimmed();
    QString additional = m_additionalEdit->text().trimmed();

    if (lastSeen.isEmpty() || uniqueDetails.isEmpty() || contents.isEmpty()) {
        m_errorLabel->setText("Please complete all required fields (*).");
        m_errorLabel->show();
        return;
    }

    if (!m_affirmationCheckbox->isChecked()) {
        m_errorLabel->setText("You must confirm the student code of conduct declaration.");
        m_errorLabel->show();
        return;
    }

    if (!m_apiClient) {
        m_errorLabel->setText("Network client unavailable.");
        m_errorLabel->show();
        return;
    }

    m_submitBtn->setEnabled(false);
    m_submitBtn->setText("Submitting Claim...");
    m_cancelBtn->setEnabled(false);

    m_apiClient->submitClaim(
        m_item.id,
        lastSeen,
        uniqueDetails,
        contents,
        additional,
        [this](const Claim& claim) {
            m_submitBtn->setEnabled(true);
            m_submitBtn->setText("Submit Claim Request");
            m_cancelBtn->setEnabled(true);

            QString refCode = claim.claimReference.trimmed();
            if (refCode.isEmpty()) {
                refCode = QString("CLM-2026-%1").arg(claim.id > 0 ? (1000 + claim.id) : (1000 + m_item.id));
            }

            QMessageBox msgBox(this);
            msgBox.setObjectName("ClaimSuccessMessageBox");
            msgBox.setWindowTitle("Claim Submitted Successfully - Campus Lost & Found");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("<h3>Claim Submitted Successfully</h3>");
            msgBox.setInformativeText(
                QString("Your ownership claim has been logged under reference:\n\n"
                        "<b>Reference Code:</b> %1\n"
                        "<b>Status:</b> Pending Admin Review\n"
                        "<b>Item:</b> %2\n\n"
                        "Our campus administrator will review your verification details. "
                        "You can track the live status under your profile / claims section.")
                    .arg(refCode)
                    .arg(m_item.title)
            );
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet(
                "QMessageBox {"
                "   background-color: #FFFFFF;"
                "}"
                "QMessageBox QLabel {"
                "   color: #0F172A;"
                "   font-family: 'Segoe UI', sans-serif;"
                "   font-size: 13px;"
                "   background-color: transparent;"
                "}"
                "QMessageBox QPushButton {"
                "   background-color: #2563EB;"
                "   color: #FFFFFF;"
                "   font-family: 'Segoe UI', sans-serif;"
                "   font-size: 13px;"
                "   font-weight: 700;"
                "   padding: 8px 24px;"
                "   border-radius: 6px;"
                "   border: none;"
                "   min-width: 80px;"
                "}"
                "QMessageBox QPushButton:hover {"
                "   background-color: #1D4ED8;"
                "}"
            );
            msgBox.exec();

            emit claimSubmitted(claim);
            accept();
        },
        [this](int statusCode, const QString& errorMessage) {
            Q_UNUSED(statusCode);
            m_submitBtn->setEnabled(true);
            m_submitBtn->setText("Submit Claim Request");
            m_cancelBtn->setEnabled(true);

            QString displayError = errorMessage.trimmed().isEmpty()
                ? "Failed to submit claim. Please try again."
                : errorMessage.trimmed();

            m_errorLabel->setText(displayError);
            m_errorLabel->show();

            QMessageBox msgBox(this);
            msgBox.setObjectName("ClaimErrorMessageBox");
            msgBox.setWindowTitle("Claim Submission Error - Campus Lost & Found");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("<h3>Unable to Submit Claim</h3>");
            msgBox.setInformativeText(displayError);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet(
                "QMessageBox {"
                "   background-color: #FFFFFF;"
                "}"
                "QMessageBox QLabel {"
                "   color: #0F172A;"
                "   font-family: 'Segoe UI', sans-serif;"
                "   font-size: 13px;"
                "   background-color: transparent;"
                "}"
                "QMessageBox QPushButton {"
                "   background-color: #EF4444;"
                "   color: #FFFFFF;"
                "   font-family: 'Segoe UI', sans-serif;"
                "   font-size: 13px;"
                "   font-weight: 700;"
                "   padding: 8px 24px;"
                "   border-radius: 6px;"
                "   border: none;"
                "   min-width: 80px;"
                "}"
                "QMessageBox QPushButton:hover {"
                "   background-color: #DC2626;"
                "}"
            );
            msgBox.exec();
        }
    );
}

