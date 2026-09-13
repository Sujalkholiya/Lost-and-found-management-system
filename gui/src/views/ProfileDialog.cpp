#include "ProfileDialog.h"
#include "../auth/AuthManager.h"
#include "../components/AvatarHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

ProfileDialog::ProfileDialog(ApiClient *apiClient, QWidget *parent)
    : QDialog(parent)
    , m_apiClient(apiClient)
    , m_avatarLabel(nullptr)
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
    setWindowTitle("User Profile Settings");
    setFixedWidth(420);
    setModal(true);

    setStyleSheet(
        "QDialog {"
        "   background-color: #FFFFFF;"
        "   font-family: 'Segoe UI', -apple-system, sans-serif;"
        "}"
        "QLabel { font-family: 'Segoe UI', -apple-system, sans-serif; }"
        "QLineEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 12px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus {"
        "   border: 1.5px solid #4F46E5;"
        "   background-color: #FFFFFF;"
        "}"
        "QLineEdit:disabled {"
        "   background-color: #F1F5F9;"
        "   color: #64748B;"
        "   border: 1px solid #E2E8F0;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 28, 28, 28);
    mainLayout->setSpacing(18);

    // Dialog Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("User Profile", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 800; color: #0F172A;");

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 14px;"
        "   color: #94A3B8;"
        "   font-weight: 700;"
        "   border-radius: 14px;"
        "}"
        "QPushButton:hover { background-color: #F1F5F9; color: #0F172A; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    mainLayout->addLayout(headerLayout);

    // Profile Card Preview (Avatar + Name Preview + Role + ID)
    QFrame *previewCard = new QFrame(this);
    previewCard->setStyleSheet(
        "QFrame {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 16px;"
        "}"
    );
    QVBoxLayout *previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setSpacing(8);
    previewLayout->setAlignment(Qt::AlignCenter);

    User currentUser = AuthManager::instance().currentUser();

    m_avatarLabel = new QLabel(previewCard);
    m_avatarLabel->setFixedSize(64, 64);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    updateAvatar(currentUser.name);
    previewLayout->addWidget(m_avatarLabel, 0, Qt::AlignCenter);

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
        m_roleBadge->setStyleSheet(
            "font-size: 10px; font-weight: 800; color: #2563EB; background-color: #EFF6FF; padding: 4px 10px; border-radius: 6px; border: 1px solid #BFDBFE;"
        );
    }

    m_userIdLabel = new QLabel(QString("ID: #%1").arg(currentUser.id), previewCard);
    m_userIdLabel->setStyleSheet(
        "font-size: 10px; font-weight: 700; color: #64748B; background-color: #E2E8F0; padding: 4px 8px; border-radius: 6px;"
    );

    badgesLayout->addWidget(m_roleBadge);
    badgesLayout->addWidget(m_userIdLabel);
    previewLayout->addLayout(badgesLayout);

    mainLayout->addWidget(previewCard);

    // Form: Email (read-only)
    QVBoxLayout *emailLayout = new QVBoxLayout();
    emailLayout->setSpacing(4);
    QLabel *emailLabel = new QLabel("CAMPUS EMAIL ADDRESS", this);
    emailLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #64748B; letter-spacing: 0.5px;");
    
    m_emailEdit = new QLineEdit(currentUser.email, this);
    m_emailEdit->setEnabled(false);
    m_emailEdit->setToolTip("Email address is managed by university account directory");

    QLabel *emailNote = new QLabel("🔒 Managed by campus identity system", this);
    emailNote->setStyleSheet("font-size: 10px; color: #94A3B8; font-style: italic;");

    emailLayout->addWidget(emailLabel);
    emailLayout->addWidget(m_emailEdit);
    emailLayout->addWidget(emailNote);
    mainLayout->addLayout(emailLayout);

    // Form: Name (editable)
    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(4);
    QLabel *nameLabel = new QLabel("FULL NAME", this);
    nameLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #64748B; letter-spacing: 0.5px;");

    m_nameEdit = new QLineEdit(currentUser.name, this);
    m_nameEdit->setPlaceholderText("Enter your full name");
    m_nameEdit->setMaxLength(100);

    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        updateAvatar(text);
        if (m_errorLabel) m_errorLabel->setVisible(false);
    });

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);

    // Error Message Label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #DC2626; font-size: 12px; font-weight: 600; padding: 4px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    mainLayout->addWidget(m_errorLabel);

    mainLayout->addSpacing(6);

    // Actions
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 10px 18px;"
        "   border-radius: 8px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_saveBtn = new QPushButton("Save Changes", this);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   padding: 10px 22px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
        "QPushButton:disabled { background-color: #A5B4FC; color: #EEF2FF; }"
    );
    connect(m_saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSaveClicked);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(buttonsLayout);
}

void ProfileDialog::updateAvatar(const QString &name)
{
    if (!m_avatarLabel) return;
    QString displayName = name.trimmed().isEmpty() ? "User" : name.trimmed();
    QPixmap avatar = AvatarHelper::createInitialsAvatar(displayName, 64, "#4F46E5");
    m_avatarLabel->setPixmap(avatar);
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

