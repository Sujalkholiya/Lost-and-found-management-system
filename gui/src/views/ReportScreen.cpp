#include "ReportScreen.h"
#include "../auth/AuthManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QShowEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QPixmap>
#include "../models/CampusLocations.h"

ReportScreen::ReportScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_authRequiredCard(nullptr)
    , m_formCard(nullptr)
    , m_titleEdit(nullptr)
    , m_typeCombo(nullptr)
    , m_locationCombo(nullptr)
    , m_customLocationEdit(nullptr)
    , m_descEdit(nullptr)
    , m_selectedImagePath(QString())
    , m_imagePreviewContainer(nullptr)
    , m_imagePreviewLabel(nullptr)
    , m_imageFileNameLabel(nullptr)
    , m_attachImageBtn(nullptr)
    , m_removeImageBtn(nullptr)
    , m_submitBtn(nullptr)
    , m_cancelBtn(nullptr)
    , m_errorLabel(nullptr)
    , m_progressBar(nullptr)
{
    setupUI();

    connect(&AuthManager::instance(), &AuthManager::authStateChanged, this, [this](bool loggedIn) {
        Q_UNUSED(loggedIn);
        checkAuthentication();
    });
}

void ReportScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    checkAuthentication();
}

void ReportScreen::checkAuthentication()
{
    bool isAuth = AuthManager::instance().isLoggedIn();
    if (m_authRequiredCard && m_formCard) {
        m_authRequiredCard->setVisible(!isAuth);
        m_formCard->setVisible(isAuth);
    }
}

void ReportScreen::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *contentWidget = new QWidget(scrollArea);
    contentWidget->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(36, 28, 36, 36);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignTop);

    // Page Title
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    QLabel *pageTitle = new QLabel("Report Lost or Found Item", contentWidget);
    pageTitle->setStyleSheet("font-size: 24px; font-weight: 800; color: #0F172A; background: transparent;");

    QLabel *pageSubtitle = new QLabel("Submit a detailed report to help return items to their rightful owners on campus.", contentWidget);
    pageSubtitle->setStyleSheet("font-size: 13px; color: #64748B; background: transparent;");

    titleLayout->addWidget(pageTitle);
    titleLayout->addWidget(pageSubtitle);
    layout->addLayout(titleLayout);

    // 1. AUTHENTICATION REQUIRED STATE CARD
    m_authRequiredCard = new QFrame(contentWidget);
    m_authRequiredCard->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "   padding: 40px;"
        "}"
    );
    QVBoxLayout *authLayout = new QVBoxLayout(m_authRequiredCard);
    authLayout->setAlignment(Qt::AlignCenter);
    authLayout->setSpacing(14);

    QLabel *lockIcon = new QLabel("🔒", m_authRequiredCard);
    lockIcon->setStyleSheet("font-size: 48px; background: transparent;");
    lockIcon->setAlignment(Qt::AlignCenter);

    QLabel *authTitle = new QLabel("You need to sign in to report an item", m_authRequiredCard);
    authTitle->setStyleSheet("font-size: 20px; font-weight: 800; color: #0F172A; background: transparent;");
    authTitle->setAlignment(Qt::AlignCenter);

    QLabel *authDesc = new QLabel("Please sign in or create an account to submit a new lost or found report on campus.", m_authRequiredCard);
    authDesc->setStyleSheet("font-size: 13px; color: #64748B; background: transparent;");
    authDesc->setAlignment(Qt::AlignCenter);

    QHBoxLayout *authBtnLayout = new QHBoxLayout();
    authBtnLayout->setAlignment(Qt::AlignCenter);
    authBtnLayout->setSpacing(12);

    QPushButton *goToLoginBtn = new QPushButton("🔑 Sign In", m_authRequiredCard);
    goToLoginBtn->setCursor(Qt::PointingHandCursor);
    goToLoginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 24px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(goToLoginBtn, &QPushButton::clicked, this, [this]() {
        emit navigateTo(3); // 3 = Login
    });

    QPushButton *goToRegisterBtn = new QPushButton("✨ Create Account", m_authRequiredCard);
    goToRegisterBtn->setCursor(Qt::PointingHandCursor);
    goToRegisterBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   font-weight: 700;"
        "   font-size: 13px;"
        "   padding: 10px 20px;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(goToRegisterBtn, &QPushButton::clicked, this, [this]() {
        emit navigateTo(4); // 4 = Register
    });

    authBtnLayout->addWidget(goToLoginBtn);
    authBtnLayout->addWidget(goToRegisterBtn);

    authLayout->addWidget(lockIcon);
    authLayout->addWidget(authTitle);
    authLayout->addWidget(authDesc);
    authLayout->addSpacing(10);
    authLayout->addLayout(authBtnLayout);

    layout->addWidget(m_authRequiredCard);

    // 2. MAIN REPORT FORM CARD
    m_formCard = new QFrame(contentWidget);
    m_formCard->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "   padding: 28px;"
        "}"
    );
    QVBoxLayout *formLayout = new QVBoxLayout(m_formCard);
    formLayout->setSpacing(18);

    // Field 1: Title
    QLabel *titleLabel = new QLabel("ITEM TITLE / NAME *", m_formCard);
    titleLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #64748B;");
    m_titleEdit = new QLineEdit(m_formCard);
    m_titleEdit->setPlaceholderText("e.g. Blue Dell XPS 15 Laptop Charger, Black Leather Wallet");
    m_titleEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 14px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus { border: 1.5px solid #4F46E5; background-color: #FFFFFF; }"
    );
    formLayout->addWidget(titleLabel);
    formLayout->addWidget(m_titleEdit);

    QString comboStyle =
        "QComboBox {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 12px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QComboBox:hover { border-color: #94A3B8; }"
        "QComboBox:focus { border: 2px solid #4F46E5; }"
        "QComboBox QAbstractItemView {"
        "   background-color: #FFFFFF;"
        "   color: #0F172A;"
        "   border: 1px solid #CBD5E1;"
        "   selection-background-color: #4F46E5;"
        "   selection-color: #FFFFFF;"
        "   outline: none;"
        "   padding: 4px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   min-height: 28px;"
        "   padding: 4px 8px;"
        "   color: #0F172A;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: #EEF2FF;"
        "   color: #4F46E5;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "}";

    // Field 2: Type Dropdown (Lost / Found)
    QLabel *typeLabel = new QLabel("REPORT TYPE *", m_formCard);
    typeLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #64748B;");
    m_typeCombo = new QComboBox(m_formCard);
    m_typeCombo->addItem("🔴 Lost (I lost this item)", "lost");
    m_typeCombo->addItem("🟢 Found (I found this item)", "found");
    m_typeCombo->setStyleSheet(comboStyle);
    formLayout->addWidget(typeLabel);
    formLayout->addWidget(m_typeCombo);

    // Field 3: Location Dropdown + Custom Input
    QLabel *locationLabel = new QLabel("CAMPUS LOCATION *", m_formCard);
    locationLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #64748B;");
    m_locationCombo = new QComboBox(m_formCard);
    for (const QString &loc : CampusLocations::predefinedLocations()) {
        m_locationCombo->addItem(loc, loc);
    }
    m_locationCombo->addItem("Other / Custom Location", "Other");
    m_locationCombo->setStyleSheet(comboStyle);

    m_customLocationEdit = new QLineEdit(m_formCard);
    m_customLocationEdit->setPlaceholderText("Enter specific room number, building, or landmark...");
    m_customLocationEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 14px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus { border: 1.5px solid #4F46E5; background-color: #FFFFFF; }"
    );
    m_customLocationEdit->hide();

    connect(m_locationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_customLocationEdit->setVisible(index == m_locationCombo->count() - 1);
    });

    formLayout->addWidget(locationLabel);
    formLayout->addWidget(m_locationCombo);
    formLayout->addWidget(m_customLocationEdit);

    // Field 4: Description
    QLabel *descLabel = new QLabel("DETAILED DESCRIPTION *", m_formCard);
    descLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #64748B;");
    m_descEdit = new QTextEdit(m_formCard);
    m_descEdit->setPlaceholderText("Provide distinguishing features (e.g. brand, color, stickers, scratches, contents, date/time lost or found)...");
    m_descEdit->setMaximumHeight(120);
    m_descEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 12px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QTextEdit:focus { border: 1.5px solid #4F46E5; background-color: #FFFFFF; }"
    );
    formLayout->addWidget(descLabel);
    formLayout->addWidget(m_descEdit);

    // Field 5: Photo Attachment (Optional)
    QLabel *photoLabel = new QLabel("ITEM PHOTO (OPTIONAL)", m_formCard);
    photoLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #64748B;");
    formLayout->addWidget(photoLabel);

    QHBoxLayout *photoControlsLayout = new QHBoxLayout();
    photoControlsLayout->setSpacing(12);

    m_attachImageBtn = new QPushButton("📷 Attach Photo", m_formCard);
    m_attachImageBtn->setCursor(Qt::PointingHandCursor);
    m_attachImageBtn->setFixedHeight(38);
    m_attachImageBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "   padding: 0 16px;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(m_attachImageBtn, &QPushButton::clicked, this, &ReportScreen::chooseImage);
    photoControlsLayout->addWidget(m_attachImageBtn);

    QLabel *photoHintLabel = new QLabel("Supports JPG, JPEG, PNG (max 5 MB)", m_formCard);
    photoHintLabel->setStyleSheet("font-size: 12px; color: #94A3B8;");
    photoControlsLayout->addWidget(photoHintLabel);
    photoControlsLayout->addStretch();
    formLayout->addLayout(photoControlsLayout);

    // Photo Preview Container (hidden by default)
    m_imagePreviewContainer = new QWidget(m_formCard);
    QHBoxLayout *previewLayout = new QHBoxLayout(m_imagePreviewContainer);
    previewLayout->setContentsMargins(0, 4, 0, 4);
    previewLayout->setSpacing(14);

    m_imagePreviewLabel = new QLabel(m_imagePreviewContainer);
    m_imagePreviewLabel->setFixedSize(120, 120);
    m_imagePreviewLabel->setAlignment(Qt::AlignCenter);
    m_imagePreviewLabel->setStyleSheet(
        "border: 1px solid #CBD5E1;"
        "border-radius: 8px;"
        "background-color: #F8FAFC;"
    );

    QVBoxLayout *previewInfoLayout = new QVBoxLayout();
    previewInfoLayout->setSpacing(8);
    previewInfoLayout->setAlignment(Qt::AlignVCenter);

    m_imageFileNameLabel = new QLabel(m_imagePreviewContainer);
    m_imageFileNameLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E293B;");

    m_removeImageBtn = new QPushButton("✕ Remove Photo", m_imagePreviewContainer);
    m_removeImageBtn->setCursor(Qt::PointingHandCursor);
    m_removeImageBtn->setFixedHeight(32);
    m_removeImageBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FEE2E2;"
        "   color: #DC2626;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "   padding: 0 14px;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 6px;"
        "}"
        "QPushButton:hover { background-color: #FECDD3; }"
    );
    connect(m_removeImageBtn, &QPushButton::clicked, this, &ReportScreen::removeImage);

    previewInfoLayout->addWidget(m_imageFileNameLabel);
    previewInfoLayout->addWidget(m_removeImageBtn);
    previewInfoLayout->addStretch();

    previewLayout->addWidget(m_imagePreviewLabel);
    previewLayout->addLayout(previewInfoLayout);
    previewLayout->addStretch();

    m_imagePreviewContainer->hide();
    formLayout->addWidget(m_imagePreviewContainer);

    // Error Message Label
    m_errorLabel = new QLabel(m_formCard);
    m_errorLabel->setStyleSheet("color: #DC2626; font-size: 12px; font-weight: 600; background: transparent;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    formLayout->addWidget(m_errorLabel);

    // Loading Progress Bar
    m_progressBar = new QProgressBar(m_formCard);
    m_progressBar->setRange(0, 0); // Animated indeterminate progress
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #E2E8F0; border-radius: 2px; border: none; }"
        "QProgressBar::chunk { background-color: #4F46E5; border-radius: 2px; }"
    );
    m_progressBar->hide();
    formLayout->addWidget(m_progressBar);

    // Actions (Submit / Cancel)
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancel", m_formCard);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setFixedHeight(44);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "   padding: 0 20px;"
        "   border-radius: 8px;"
        "   border: 1px solid #E2E8F0;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; }"
    );
    connect(m_cancelBtn, &QPushButton::clicked, this, [this]() {
        resetForm();
        emit navigateTo(1); // Return to Browse
    });

    m_submitBtn = new QPushButton("Submit Item Report", m_formCard);
    m_submitBtn->setCursor(Qt::PointingHandCursor);
    m_submitBtn->setFixedHeight(44);
    m_submitBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 14px;"
        "   padding: 0 24px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
        "QPushButton:disabled { background-color: #A5B4FC; }"
    );
    connect(m_submitBtn, &QPushButton::clicked, this, &ReportScreen::submitReport);

    actionsLayout->addStretch();
    actionsLayout->addWidget(m_cancelBtn);
    actionsLayout->addWidget(m_submitBtn);
    formLayout->addLayout(actionsLayout);

    layout->addWidget(m_formCard);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);

    checkAuthentication();
}

void ReportScreen::setReportType(const QString &type)
{
    if (type.toLower() == "found") {
        m_typeCombo->setCurrentIndex(1);
    } else {
        m_typeCombo->setCurrentIndex(0);
    }
}

void ReportScreen::setLoading(bool loading)
{
    m_submitBtn->setEnabled(!loading);
    m_cancelBtn->setEnabled(!loading);
    m_progressBar->setVisible(loading);
    if (loading) {
        m_submitBtn->setText("Submitting Report...");
    } else {
        m_submitBtn->setText("Submit Item Report");
    }
}

void ReportScreen::showValidationError(const QString &error)
{
    m_errorLabel->setText("⚠️ " + error);
    m_errorLabel->show();
}

void ReportScreen::chooseImage()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Item Photo",
        QString(),
        "Images (*.png *.jpg *.jpeg)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    constexpr qint64 MAX_IMAGE_BYTES = 5 * 1024 * 1024; // 5 MB

    if (fileInfo.size() > MAX_IMAGE_BYTES) {
        showValidationError("Selected image exceeds maximum allowed size of 5 MB.");
        return;
    }

    QPixmap previewPixmap(filePath);
    if (previewPixmap.isNull()) {
        showValidationError("Failed to load the selected image. Please choose a valid image file.");
        return;
    }

    m_selectedImagePath = filePath;
    m_imagePreviewLabel->setPixmap(previewPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_imageFileNameLabel->setText(QString("📎 %1 (%2 KB)").arg(fileInfo.fileName()).arg(fileInfo.size() / 1024));
    m_imagePreviewContainer->show();
    m_attachImageBtn->setText("🔄 Change Photo");
    m_errorLabel->hide();
}

void ReportScreen::removeImage()
{
    m_selectedImagePath.clear();
    m_imagePreviewLabel->clear();
    m_imageFileNameLabel->clear();
    m_imagePreviewContainer->hide();
    m_attachImageBtn->setText("📷 Attach Photo");
}

void ReportScreen::resetForm()
{
    m_titleEdit->clear();
    m_descEdit->clear();
    m_customLocationEdit->clear();
    m_locationCombo->setCurrentIndex(0);
    m_typeCombo->setCurrentIndex(0);
    removeImage();
    m_errorLabel->hide();
    setLoading(false);
}

void ReportScreen::submitReport()
{
    // 1. Guard check authentication
    if (!AuthManager::instance().isLoggedIn()) {
        emit showToast("Please sign in before submitting a report.", ToastType::Warning);
        emit navigateTo(3); // Navigate to Login Screen
        return;
    }

    // 2. Input Validation
    QString title = m_titleEdit->text().trimmed();
    QString desc = m_descEdit->toPlainText().trimmed();
    QString type = m_typeCombo->currentData().toString();
    QString location;

    if (m_locationCombo->currentIndex() == m_locationCombo->count() - 1) {
        location = m_customLocationEdit->text().trimmed();
    } else {
        location = m_locationCombo->currentText();
    }

    if (title.isEmpty()) {
        showValidationError("Item title is required.");
        m_titleEdit->setFocus();
        return;
    }

    if (title.length() < 3) {
        showValidationError("Item title must be at least 3 characters long.");
        m_titleEdit->setFocus();
        return;
    }

    if (location.isEmpty()) {
        showValidationError("Campus location is required.");
        m_locationCombo->setFocus();
        return;
    }

    if (desc.isEmpty()) {
        showValidationError("Detailed description is required to identify the item.");
        m_descEdit->setFocus();
        return;
    }

    m_errorLabel->hide();
    setLoading(true);

    // 3. API Call via centralized ApiClient (sends Bearer token and optional image attachment)
    m_apiClient->createItem(
        title,
        desc,
        type,
        location,
        [this](const Item &item) {
            Q_UNUSED(item);
            setLoading(false);
            resetForm();
            emit showToast("Item report submitted successfully!", ToastType::Success);
            // 4. Navigate back to Browse Items Screen
            emit navigateTo(1); // 1 = Browse
        },
        [this](int statusCode, const QString &errorMessage) {
            setLoading(false);
            if (statusCode == 401) {
                emit showToast("Your session has expired. Please sign in again.", ToastType::Error);
                emit navigateTo(3); // 3 = Login
            } else {
                m_errorLabel->setText("❌ Failed to create report: " + errorMessage);
                m_errorLabel->show();
            }
        },
        m_selectedImagePath
    );
}
