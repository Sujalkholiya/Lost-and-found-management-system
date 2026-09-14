#include "ProfileDialog.h"
#include "../auth/AuthManager.h"
#include "../components/AvatarHelper.h"
#include "../components/Theme.h"
#include "../components/ProfileImageManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QMessageBox>

ProfileDialog::ProfileDialog(ApiClient *apiClient, QWidget *parent)
    : QDialog(parent)
    , m_apiClient(apiClient)
    , m_avatarLabel(nullptr)
    , m_changePhotoBtn(nullptr)
    , m_removePhotoBtn(nullptr)
    , m_roleBadge(nullptr)
    , m_userIdLabel(nullptr)
    , m_nameEdit(nullptr)
    , m_emailEdit(nullptr)
    , m_errorLabel(nullptr)
    , m_saveBtn(nullptr)
    , m_cancelBtn(nullptr)
{
    setupUI();
}

void ProfileDialog::setupUI()
{
    setObjectName("ProfileDialog");
    setWindowTitle("Profile Settings");
    setFixedWidth(430);
    setModal(true);

    setStyleSheet(QString(
        "#ProfileDialog {"
        "   background-color: %1;"
        "   font-family: 'Segoe UI', -apple-system, sans-serif;"
        "}"
        "#ProfileDialog QLabel { font-family: 'Segoe UI', -apple-system, sans-serif; }"
        "#ProfileDialog QLineEdit {"
        "   background-color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: 8px;"
        "   padding: 10px 12px;"
        "   font-size: 13px;"
        "   color: %4;"
        "}"
        "#ProfileDialog QLineEdit:focus {"
        "   border: 1.5px solid %5;"
        "   background-color: #FFFFFF;"
        "}"
        "#ProfileDialog QLineEdit:disabled {"
        "   background-color: %2;"
        "   color: %6;"
        "   border: 1px solid %3;"
        "}"
    ).arg(Theme::Background::Card,
          Theme::Background::Muted,
          Theme::Border::Default,
          Theme::Text::Primary,
          Theme::Primary::Main,
          Theme::Text::Muted));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 28, 28, 28);
    mainLayout->setSpacing(18);

    // Dialog Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Profile Settings", this);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 800; color: %1;").arg(Theme::Text::Primary));

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 14px;"
        "   color: %1;"
        "   font-weight: 700;"
        "   border-radius: 14px;"
        "}"
        "QPushButton:hover { background-color: %2; color: %3; }"
    ).arg(Theme::Text::Muted, Theme::Background::Muted, Theme::Text::Primary));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    mainLayout->addLayout(headerLayout);

    // Profile Card Preview (Avatar + Photo Actions + Name Preview + Role + ID)
    QFrame *previewCard = new QFrame(this);
    previewCard->setStyleSheet(QString(
        "QFrame {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: 12px;"
        "   padding: 16px;"
        "}"
    ).arg(Theme::Background::Muted, Theme::Border::Default));
    QVBoxLayout *previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setSpacing(10);
    previewLayout->setAlignment(Qt::AlignCenter);

    User currentUser = AuthManager::instance().currentUser();

    m_avatarLabel = new QLabel(previewCard);
    m_avatarLabel->setFixedSize(64, 64);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    previewLayout->addWidget(m_avatarLabel, 0, Qt::AlignCenter);

    // Photo Action Buttons
    QHBoxLayout *photoBtnLayout = new QHBoxLayout();
    photoBtnLayout->setSpacing(8);
    photoBtnLayout->setAlignment(Qt::AlignCenter);

    m_changePhotoBtn = new QPushButton("📷 Change Photo", previewCard);
    m_changePhotoBtn->setCursor(Qt::PointingHandCursor);
    m_changePhotoBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   padding: 6px 14px;"
        "   border-radius: 6px;"
        "   border: 1px solid %3;"
        "}"
        "QPushButton:hover { background-color: %4; border-color: %5; color: %5; }"
    ).arg(Theme::Background::Card, Theme::Primary::Main, Theme::Primary::Border, Theme::Primary::Light, Theme::Primary::Dark));
    connect(m_changePhotoBtn, &QPushButton::clicked, this, &ProfileDialog::onChangePhotoClicked);

    m_removePhotoBtn = new QPushButton("🗑️ Remove Photo", previewCard);
    m_removePhotoBtn->setCursor(Qt::PointingHandCursor);
    m_removePhotoBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFF7F7;"
        "   color: #EF4444;"
        "   font-size: 11px;"
        "   font-weight: 600;"
        "   padding: 6px 12px;"
        "   border-radius: 6px;"
        "   border: 1px solid #FECDD3;"
        "}"
        "QPushButton:hover { background-color: #FEE2E2; color: #DC2626; border-color: #FCA5A5; }"
        "QPushButton:disabled { background-color: #F8FAFC; color: #94A3B8; border-color: #E2E8F0; }"
    );
    connect(m_removePhotoBtn, &QPushButton::clicked, this, &ProfileDialog::onRemovePhotoClicked);

    photoBtnLayout->addWidget(m_changePhotoBtn);
    photoBtnLayout->addWidget(m_removePhotoBtn);
    previewLayout->addLayout(photoBtnLayout);

    refreshAvatar(currentUser.name);

    QHBoxLayout *badgesLayout = new QHBoxLayout();
    badgesLayout->setSpacing(8);
    badgesLayout->setAlignment(Qt::AlignCenter);

    m_roleBadge = new QLabel(previewCard);
    if (currentUser.isAdmin()) {
        m_roleBadge->setText("👑 ADMINISTRATOR");
        m_roleBadge->setStyleSheet(
            "font-size: 10px; font-weight: 800; color: #7C3AED; background-color: #F5F3FF; padding: 4px 10px; border-radius: 6px; border: 1px solid #DDD6FE;"
        );
    } else {
        m_roleBadge->setText("🎓 STUDENT");
        m_roleBadge->setStyleSheet(QString(
            "font-size: 10px; font-weight: 800; color: %1; background-color: %2; padding: 4px 10px; border-radius: 6px; border: 1px solid %3;"
        ).arg(Theme::Primary::Main, Theme::Primary::Light, Theme::Border::Default));
    }

    m_userIdLabel = new QLabel(QString("ID: #%1").arg(currentUser.id), previewCard);
    m_userIdLabel->setStyleSheet(QString(
        "font-size: 10px; font-weight: 700; color: %1; background-color: %2; padding: 4px 8px; border-radius: 6px; border: 1px solid %3;"
    ).arg(Theme::Text::Secondary, Theme::Background::Card, Theme::Border::Default));

    badgesLayout->addWidget(m_roleBadge);
    badgesLayout->addWidget(m_userIdLabel);
    previewLayout->addLayout(badgesLayout);

    mainLayout->addWidget(previewCard);

    // Form: Email (read-only)
    QVBoxLayout *emailLayout = new QVBoxLayout();
    emailLayout->setSpacing(4);
    QLabel *emailLabel = new QLabel("CAMPUS EMAIL ADDRESS", this);
    emailLabel->setStyleSheet(QString("font-size: 11px; font-weight: 700; color: %1; letter-spacing: 0.5px;").arg(Theme::Text::Secondary));
    
    m_emailEdit = new QLineEdit(currentUser.email, this);
    m_emailEdit->setEnabled(false);
    m_emailEdit->setToolTip("Email address is managed by university account directory");

    QLabel *emailNote = new QLabel("🔒 Managed by campus identity system", this);
    emailNote->setStyleSheet(QString("font-size: 10px; color: %1; font-style: italic;").arg(Theme::Text::Muted));

    emailLayout->addWidget(emailLabel);
    emailLayout->addWidget(m_emailEdit);
    emailLayout->addWidget(emailNote);
    mainLayout->addLayout(emailLayout);

    // Form: Name (editable)
    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(4);
    QLabel *nameLabel = new QLabel("FULL NAME", this);
    nameLabel->setStyleSheet(QString("font-size: 11px; font-weight: 700; color: %1; letter-spacing: 0.5px;").arg(Theme::Text::Secondary));

    m_nameEdit = new QLineEdit(currentUser.name, this);
    m_nameEdit->setPlaceholderText("Enter your full name");
    m_nameEdit->setMaxLength(100);

    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        refreshAvatar(text);
        if (m_errorLabel) m_errorLabel->setVisible(false);
    });

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);

    // Error Message Label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; padding: 4px;").arg(Theme::Status::ErrorText));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    mainLayout->addWidget(m_errorLabel);

    mainLayout->addSpacing(6);

    // Actions
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: 1px solid %3;"
        "}"
        "QPushButton:hover { background-color: %4; color: %5; }"
    ).arg(Theme::Background::Card, Theme::Text::Secondary, Theme::Border::Default, Theme::Background::Muted, Theme::Text::Primary));
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_saveBtn = new QPushButton("Save Changes", this);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: #FFFFFF;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 10px 22px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: #93C5FD; color: #FFFFFF; }"
    ).arg(Theme::Primary::Main, Theme::Primary::Hover));
    connect(m_saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSaveClicked);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(buttonsLayout);
}

void ProfileDialog::refreshAvatar(const QString &name)
{
    if (!m_avatarLabel) return;
    User currentUser = AuthManager::instance().currentUser();
    QString displayName = name.trimmed().isEmpty() ? currentUser.name : name.trimmed();
    QPixmap avatar = ProfileImageManager::instance().getCircularAvatar(currentUser.id, displayName, 64, Theme::Primary::Main);
    m_avatarLabel->setPixmap(avatar);

    if (m_removePhotoBtn) {
        m_removePhotoBtn->setEnabled(ProfileImageManager::instance().hasCustomAvatar(currentUser.id));
    }
}

void ProfileDialog::onChangePhotoClicked()
{
    User currentUser = AuthManager::instance().currentUser();
    if (currentUser.id <= 0) {
        QMessageBox::warning(this, "Profile Error", "User session is invalid. Please sign in again.");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Profile Image",
        QString(),
        "Images (*.png *.jpg *.jpeg *.webp)"
    );

    if (filePath.isEmpty()) {
        return; // User canceled file picker
    }

    QString errorMsg;
    bool success = ProfileImageManager::instance().saveProfileImage(currentUser.id, filePath, errorMsg);
    if (!success) {
        QMessageBox::warning(
            this,
            "Invalid Image",
            errorMsg.isEmpty() ? "Unable to load this image. Please select a valid PNG, JPG, JPEG, or WEBP image." : errorMsg
        );
        return;
    }

    refreshAvatar(m_nameEdit ? m_nameEdit->text() : currentUser.name);
}

void ProfileDialog::onRemovePhotoClicked()
{
    User currentUser = AuthManager::instance().currentUser();
    if (currentUser.id <= 0) return;

    if (!ProfileImageManager::instance().hasCustomAvatar(currentUser.id)) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Remove Profile Image?",
        "Your current profile image will be removed. Are you sure?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        ProfileImageManager::instance().removeProfileImage(currentUser.id);
        refreshAvatar(m_nameEdit ? m_nameEdit->text() : currentUser.name);
    }
}

void ProfileDialog::onSaveClicked()
{
    QString newName = m_nameEdit->text().trimmed();

    if (newName.isEmpty()) {
        m_errorLabel->setText("Full name cannot be empty.");
        m_errorLabel->setVisible(true);
        m_nameEdit->setFocus();
        return;
    }

    if (newName.length() > 100) {
        m_errorLabel->setText("Full name must be under 100 characters.");
        m_errorLabel->setVisible(true);
        m_nameEdit->setFocus();
        return;
    }

    m_saveBtn->setEnabled(false);
    m_saveBtn->setText("Saving...");
    m_cancelBtn->setEnabled(false);
    m_errorLabel->setVisible(false);

    m_apiClient->updateProfile(
        newName,
        [this, newName](const QJsonObject &response) {
            Q_UNUSED(response);
            AuthManager::instance().updateProfile(newName);
            emit profileUpdated(newName);
            accept();
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            m_saveBtn->setEnabled(true);
            m_saveBtn->setText("Save Changes");
            m_cancelBtn->setEnabled(true);
            m_errorLabel->setText(errorMessage.isEmpty() ? "Failed to update profile." : errorMessage);
            m_errorLabel->setVisible(true);
        }
    );
}

