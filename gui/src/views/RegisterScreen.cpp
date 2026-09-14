#include "RegisterScreen.h"
#include "../components/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QRegularExpression>

RegisterScreen::RegisterScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_nameEdit(nullptr)
    , m_emailEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_confirmPassEdit(nullptr)
    , m_phoneEdit(nullptr)
    , m_showPassBtn(nullptr)
    , m_registerBtn(nullptr)
    , m_loginLink(nullptr)
    , m_backBtn(nullptr)
    , m_errorLabel(nullptr)
{
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setContentsMargins(24, 32, 24, 32);

    QFrame *card = new QFrame(scrollContent);
    card->setFixedWidth(460);
    card->setStyleSheet(QString(
        "QFrame#registerCard {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: 18px;"
        "   padding: 32px;"
        "}"
    ).arg(Theme::Background::Card, Theme::Border::Default));
    card->setObjectName("registerCard");

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(14);

    // Top Navigation: Back to Home
    m_backBtn = new QPushButton("← Back to Home", card);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background: transparent;"
        "   color: %1;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   border: none;"
        "   text-align: left;"
        "   padding: 0;"
        "}"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::Text::Secondary, Theme::Primary::Main));
    connect(m_backBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(0); });
    layout->addWidget(m_backBtn);
    layout->addSpacing(2);

    // Header Branding & Title
    QLabel *icon = new QLabel("🎓", card);
    icon->setStyleSheet("font-size: 38px; background: transparent;");
    icon->setAlignment(Qt::AlignCenter);

    QLabel *title = new QLabel("Create Your Account", card);
    title->setStyleSheet(QString("font-size: 24px; font-weight: 800; color: %1; background: transparent;").arg(Theme::Text::Primary));
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Join the Campus Lost & Found Network.", card);
    subtitle->setStyleSheet(QString("font-size: 13px; color: %1; background: transparent;").arg(Theme::Text::Secondary));
    subtitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(6);

    QString inputStyle = QString(
        "QLineEdit {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: 8px;"
        "   padding: 10px 14px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   selection-background-color: %4;"
        "   selection-color: #FFFFFF;"
        "}"
        "QLineEdit:hover { border-color: #CBD5E1; }"
        "QLineEdit:focus {"
        "   border: 2px solid %4;"
        "   background-color: #FFFFFF;"
        "   color: %2;"
        "}"
    ).arg(Theme::Background::Card, Theme::Text::Primary, Theme::Border::Default, Theme::Primary::Main);

    // 1. Full Name
    QVBoxLayout *nameBlock = new QVBoxLayout();
    nameBlock->setSpacing(6);
    QLabel *nameLabel = new QLabel("Full Name", card);
    nameLabel->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1; background: transparent;").arg(Theme::Text::Primary));
    m_nameEdit = new QLineEdit(card);
    m_nameEdit->setPlaceholderText("Enter your full name");
    m_nameEdit->setStyleSheet(inputStyle);
    nameBlock->addWidget(nameLabel);
    nameBlock->addWidget(m_nameEdit);
    layout->addLayout(nameBlock);

    // 2. Email
    QVBoxLayout *emailBlock = new QVBoxLayout();
    emailBlock->setSpacing(6);
    QLabel *emailLabel = new QLabel("Email", card);
    emailLabel->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1; background: transparent;").arg(Theme::Text::Primary));
    m_emailEdit = new QLineEdit(card);
    m_emailEdit->setPlaceholderText("Enter your college email");
    m_emailEdit->setStyleSheet(inputStyle);
    emailBlock->addWidget(emailLabel);
    emailBlock->addWidget(m_emailEdit);
    layout->addLayout(emailBlock);

    // 3. Password
    QVBoxLayout *passBlock = new QVBoxLayout();
    passBlock->setSpacing(4);
    QLabel *passLabel = new QLabel("Password", card);
    passLabel->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1; background: transparent;").arg(Theme::Text::Primary));
    
    QWidget *passContainer = new QWidget(card);
    QHBoxLayout *passLayout = new QHBoxLayout(passContainer);
    passLayout->setContentsMargins(0, 0, 0, 0);
    passLayout->setSpacing(8);

    m_passwordEdit = new QLineEdit(passContainer);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Create a password");
    m_passwordEdit->setStyleSheet(inputStyle);

    m_showPassBtn = new QPushButton("Show", passContainer);
    m_showPassBtn->setCursor(Qt::PointingHandCursor);
    m_showPassBtn->setFixedWidth(64);
    m_showPassBtn->setFixedHeight(40);
    m_showPassBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   border: 1px solid %3;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: %4; color: %5; }"
    ).arg(Theme::Background::Card, Theme::Text::Secondary, Theme::Border::Default, Theme::Background::Muted, Theme::Text::Primary));
    connect(m_showPassBtn, &QPushButton::clicked, this, &RegisterScreen::togglePasswordVisibility);

    passLayout->addWidget(m_passwordEdit, 1);
    passLayout->addWidget(m_showPassBtn);

    QLabel *passHint = new QLabel("8+ characters • 1 letter • 1 number", card);
    passHint->setStyleSheet(QString("font-size: 11px; font-weight: 500; color: %1; background: transparent; margin-top: 2px;").arg(Theme::Text::Muted));

    passBlock->addWidget(passLabel);
    passBlock->addWidget(passContainer);
    passBlock->addWidget(passHint);
    layout->addLayout(passBlock);

    // 4. Confirm Password
    QVBoxLayout *confBlock = new QVBoxLayout();
    confBlock->setSpacing(6);
    QLabel *confLabel = new QLabel("Confirm Password", card);
    confLabel->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1; background: transparent;").arg(Theme::Text::Primary));
    m_confirmPassEdit = new QLineEdit(card);
    m_confirmPassEdit->setEchoMode(QLineEdit::Password);
    m_confirmPassEdit->setPlaceholderText("Re-enter your password");
    m_confirmPassEdit->setStyleSheet(inputStyle);
    confBlock->addWidget(confLabel);
    confBlock->addWidget(m_confirmPassEdit);
    layout->addLayout(confBlock);

    // 5. Phone Number (Optional)
    QVBoxLayout *phoneBlock = new QVBoxLayout();
    phoneBlock->setSpacing(6);
    QLabel *phoneLabel = new QLabel("Phone Number (Optional)", card);
    phoneLabel->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1; background: transparent;").arg(Theme::Text::Primary));
    m_phoneEdit = new QLineEdit(card);
    m_phoneEdit->setPlaceholderText("+91 XXXXX XXXXX");
    m_phoneEdit->setStyleSheet(inputStyle);
    phoneBlock->addWidget(phoneLabel);
    phoneBlock->addWidget(m_phoneEdit);
    layout->addLayout(phoneBlock);

    // 6. Error Alert Box
    m_errorLabel = new QLabel(card);
    m_errorLabel->setStyleSheet(QString(
        "QLabel {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 8px;"
        "   padding: 10px 14px;"
        "   font-size: 12px;"
        "   font-weight: 600;"
        "}"
    ).arg(Theme::Status::ErrorBg, Theme::Status::ErrorText));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    layout->addWidget(m_errorLabel);

    // 7. Create Account Button
    m_registerBtn = new QPushButton("Create Account", card);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setFixedHeight(44);
    m_registerBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: #FFFFFF;"
        "   font-weight: 700;"
        "   font-size: 14px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: #93C5FD; color: #FFFFFF; }"
    ).arg(Theme::Primary::Main, Theme::Primary::Dark));
    connect(m_registerBtn, &QPushButton::clicked, this, &RegisterScreen::handleRegister);
    connect(m_confirmPassEdit, &QLineEdit::returnPressed, this, &RegisterScreen::handleRegister);
    connect(m_phoneEdit, &QLineEdit::returnPressed, this, &RegisterScreen::handleRegister);
    layout->addWidget(m_registerBtn);

    // 8. Login Link Footer
    QHBoxLayout *footLayout = new QHBoxLayout();
    QLabel *footText = new QLabel("Already have an account?", card);
    footText->setStyleSheet(QString("font-size: 13px; color: %1; background: transparent;").arg(Theme::Text::Secondary));
    
    m_loginLink = new QPushButton("Sign In", card);
    m_loginLink->setCursor(Qt::PointingHandCursor);
    m_loginLink->setStyleSheet(QString(
        "QPushButton {"
        "   background: transparent;"
        "   color: %1;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   border: none;"
        "}"
        "QPushButton:hover { text-decoration: underline; }"
    ).arg(Theme::Primary::Main));
    connect(m_loginLink, &QPushButton::clicked, this, [this]() { emit navigateTo(3); }); // 3 = Login

    footLayout->addStretch();
    footLayout->addWidget(footText);
    footLayout->addWidget(m_loginLink);
    footLayout->addStretch();
    layout->addLayout(footLayout);

    contentLayout->addWidget(card);
    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);
}

void RegisterScreen::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_confirmPassEdit->setEchoMode(QLineEdit::Normal);
        m_showPassBtn->setText("Hide");
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_confirmPassEdit->setEchoMode(QLineEdit::Password);
        m_showPassBtn->setText("Show");
    }
}

void RegisterScreen::resetForm()
{
    m_nameEdit->clear();
    m_emailEdit->clear();
    m_passwordEdit->clear();
    m_confirmPassEdit->clear();
    m_phoneEdit->clear();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPassEdit->setEchoMode(QLineEdit::Password);
    m_showPassBtn->setText("Show");
    m_errorLabel->hide();
}

void RegisterScreen::handleRegister()
{
    QString name = m_nameEdit->text().trimmed();
    QString email = m_emailEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    QString confirmPass = m_confirmPassEdit->text();
    QString phone = m_phoneEdit->text().trimmed();

    // 1. Required fields check
    if (name.isEmpty() || email.isEmpty() || pass.isEmpty() || confirmPass.isEmpty()) {
        m_errorLabel->setText("⚠️ Name, email, and password fields are required.");
        m_errorLabel->show();
        return;
    }

    // 2. Email validation
    static const QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        m_errorLabel->setText("⚠️ Please enter a valid email address.");
        m_errorLabel->show();
        return;
    }

    // 3. Password validation (8+ chars, 1 letter, 1 number)
    if (pass.length() < 8) {
        m_errorLabel->setText("⚠️ Password must be at least 8 characters long.");
        m_errorLabel->show();
        return;
    }
    bool hasLetter = false;
    bool hasDigit = false;
    for (const QChar &ch : pass) {
        if (ch.isLetter()) hasLetter = true;
        if (ch.isDigit()) hasDigit = true;
    }
    if (!hasLetter || !hasDigit) {
        m_errorLabel->setText("⚠️ Password must contain at least 1 letter and 1 number.");
        m_errorLabel->show();
        return;
    }

    // 4. Confirm Password Match
    if (pass != confirmPass) {
        m_errorLabel->setText("⚠️ Passwords do not match.");
        m_errorLabel->show();
        return;
    }

    // 5. Phone validation (if provided)
    if (!phone.isEmpty()) {
        QString cleanPhone = phone;
        cleanPhone.remove(' ');
        cleanPhone.remove('-');
        cleanPhone.remove('(');
        cleanPhone.remove(')');
        if (cleanPhone.startsWith("+91")) {
            cleanPhone = cleanPhone.mid(3);
        } else if (cleanPhone.startsWith("91") && cleanPhone.length() == 12) {
            cleanPhone = cleanPhone.mid(2);
        } else if (cleanPhone.startsWith("0") && cleanPhone.length() == 11) {
            cleanPhone = cleanPhone.mid(1);
        }

        static const QRegularExpression phoneRegex(R"(^[6-9]\d{9}$)");
        if (!phoneRegex.match(cleanPhone).hasMatch()) {
            m_errorLabel->setText("⚠️ Please enter a valid 10-digit Indian mobile number.");
            m_errorLabel->show();
            return;
        }
    }

    m_errorLabel->hide();
    m_registerBtn->setEnabled(false);
    m_registerBtn->setText("Creating Account...");

    // Send only name, email, password to backend as documented
    m_apiClient->registerUser(name, email, pass,
        [this](const QJsonObject &response) {
            Q_UNUSED(response);
            m_registerBtn->setEnabled(true);
            m_registerBtn->setText("Create Account");
            m_errorLabel->hide();
            resetForm();
            emit showToast("Account created successfully. Please sign in.", ToastType::Success);
            emit navigateTo(3); // Redirect to Login
        },
        [this](int statusCode, const QString &errorMessage) {
            Q_UNUSED(statusCode);
            m_registerBtn->setEnabled(true);
            m_registerBtn->setText("Create Account");
            m_errorLabel->setText("❌ " + errorMessage);
            m_errorLabel->show();
        }
    );
}