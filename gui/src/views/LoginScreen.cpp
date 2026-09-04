#include "LoginScreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QRegularExpression>
#include <QDebug>

LoginScreen::LoginScreen(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent)
    , m_apiClient(apiClient)
    , m_emailEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_showPassBtn(nullptr)
    , m_loginBtn(nullptr)
    , m_registerLink(nullptr)
    , m_backBtn(nullptr)
    , m_errorLabel(nullptr)
{
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: #F8FAFC;");

    QWidget *scrollContent = new QWidget(scrollArea);
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setContentsMargins(24, 32, 24, 32);

    QFrame *card = new QFrame(scrollContent);
    card->setFixedWidth(440);
    card->setStyleSheet(
        "QFrame#loginCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "   padding: 32px;"
        "}"
    );
    card->setObjectName("loginCard");

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(16);

    // Top Navigation: Back to Home
    m_backBtn = new QPushButton("← Back to Home", card);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #64748B;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   border: none;"
        "   text-align: left;"
        "   padding: 0;"
        "}"
        "QPushButton:hover { color: #4F46E5; }"
    );
    connect(m_backBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(0); });
    layout->addWidget(m_backBtn);
    layout->addSpacing(4);

    // Header Branding & Title
    QLabel *icon = new QLabel("🎓", card);
    icon->setStyleSheet("font-size: 38px; background: transparent;");
    icon->setAlignment(Qt::AlignCenter);

    QLabel *title = new QLabel("Welcome Back", card);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #0F172A; background: transparent;");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Sign in to your Campus Lost & Found account.", card);
    subtitle->setStyleSheet("font-size: 13px; color: #64748B; background: transparent;");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);

    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(6);

    QString inputStyle =
        "QLineEdit {"
        "   background-color: #FFFFFF;"
        "   color: #0F172A;"
        "   border: 1.5px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 14px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   selection-background-color: #4F46E5;"
        "   selection-color: #FFFFFF;"
        "}"
        "QLineEdit:hover { border-color: #94A3B8; }"
        "QLineEdit:focus {"
        "   border: 2px solid #4F46E5;"
        "   background-color: #FFFFFF;"
        "   color: #0F172A;"
        "}";

    // 1. Email Field
    QVBoxLayout *emailBlock = new QVBoxLayout();
    emailBlock->setSpacing(6);
    QLabel *emailLabel = new QLabel("Email", card);
    emailLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #334155; background: transparent;");
    
    m_emailEdit = new QLineEdit(card);
    m_emailEdit->setPlaceholderText("Enter your email");
    m_emailEdit->setStyleSheet(inputStyle);
    emailBlock->addWidget(emailLabel);
    emailBlock->addWidget(m_emailEdit);
    layout->addLayout(emailBlock);

    // 2. Password Field with Show/Hide toggle
    QVBoxLayout *passBlock = new QVBoxLayout();
    passBlock->setSpacing(6);
    QLabel *passLabel = new QLabel("Password", card);
    passLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #334155; background: transparent;");

    QWidget *passContainer = new QWidget(card);
    QHBoxLayout *passLayout = new QHBoxLayout(passContainer);
    passLayout->setContentsMargins(0, 0, 0, 0);
    passLayout->setSpacing(8);

    m_passwordEdit = new QLineEdit(passContainer);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter your password");
    m_passwordEdit->setStyleSheet(inputStyle);

    m_showPassBtn = new QPushButton("Show", passContainer);
    m_showPassBtn->setCursor(Qt::PointingHandCursor);
    m_showPassBtn->setFixedWidth(64);
    m_showPassBtn->setFixedHeight(40);
    m_showPassBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   border: 1.5px solid #CBD5E1;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
    );
    connect(m_showPassBtn, &QPushButton::clicked, this, &LoginScreen::togglePasswordVisibility);

    passLayout->addWidget(m_passwordEdit, 1);
    passLayout->addWidget(m_showPassBtn);

    passBlock->addWidget(passLabel);
    passBlock->addWidget(passContainer);
    layout->addLayout(passBlock);

    // 3. Error Alert Box
    m_errorLabel = new QLabel(card);
    m_errorLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #FEF2F2;"
        "   color: #DC2626;"
        "   border: 1px solid #FECDD3;"
        "   border-radius: 8px;"
        "   padding: 10px 14px;"
        "   font-size: 12px;"
        "   font-weight: 600;"
        "}"
    );
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    layout->addWidget(m_errorLabel);

    // 4. Sign In Button
    m_loginBtn = new QPushButton("Sign In", card);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setFixedHeight(44);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-weight: 800;"
        "   font-size: 14px;"
        "   border-radius: 8px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
        "QPushButton:disabled { background-color: #A5B4FC; color: #FFFFFF; }"
    );
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginScreen::handleLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginScreen::handleLogin);
    connect(m_emailEdit, &QLineEdit::returnPressed, this, &LoginScreen::handleLogin);
    layout->addWidget(m_loginBtn);

    // 5. Register Link Footer
    QHBoxLayout *footLayout = new QHBoxLayout();
    QLabel *footText = new QLabel("Don't have an account?", card);
    footText->setStyleSheet("font-size: 13px; color: #64748B; background: transparent;");
    
    m_registerLink = new QPushButton("Create Account", card);
    m_registerLink->setCursor(Qt::PointingHandCursor);
    m_registerLink->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #4F46E5;"
        "   font-size: 13px;"
        "   font-weight: 800;"
        "   border: none;"
        "}"
        "QPushButton:hover { text-decoration: underline; }"
    );
    connect(m_registerLink, &QPushButton::clicked, this, [this]() { emit navigateTo(4); }); // 4 = Register

    footLayout->addStretch();
    footLayout->addWidget(footText);
    footLayout->addWidget(m_registerLink);
    footLayout->addStretch();
    layout->addLayout(footLayout);

    contentLayout->addWidget(card);
    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);
}

void LoginScreen::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_showPassBtn->setText("Hide");
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_showPassBtn->setText("Show");
    }
}

void LoginScreen::resetForm()
{
    m_emailEdit->clear();
    m_passwordEdit->clear();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_showPassBtn->setText("Show");
    m_errorLabel->hide();
}

void LoginScreen::handleLogin()
{
    QString email = m_emailEdit->text().trimmed();
    QString pass = m_passwordEdit->text();

    qDebug() << "[LoginScreen] Sign In button clicked.";
    qDebug() << "[LoginScreen] Submitting email:" << email;

    // 1. Required fields validation
    if (email.isEmpty() || pass.isEmpty()) {
        qDebug() << "[LoginScreen] Validation rejected: Email or password is empty.";
        m_errorLabel->setText("⚠️ Email and password are required.");
        m_errorLabel->show();
        return;
    }

    // 2. Email format validation
    static const QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        qDebug() << "[LoginScreen] Validation rejected: Invalid email format for" << email;
        m_errorLabel->setText("⚠️ Please enter a valid email address.");
        m_errorLabel->show();
        return;
    }

    m_errorLabel->hide();
    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("Signing In...");

    qDebug() << "[LoginScreen] Dispatching login request to ApiClient...";

    m_apiClient->loginUser(email, pass,
        [this](const User &user, const QString &token) {
            Q_UNUSED(token);
            qDebug() << "[LoginScreen] Login success callback received. User ID:" << user.id
                     << "Role:" << user.role;
            m_loginBtn->setEnabled(true);
            m_loginBtn->setText("Sign In");
            m_errorLabel->hide();
            resetForm();

            // Navigate to appropriate dashboard based on user role
            int targetScreen = user.isAdmin() ? 6 : 5;
            qDebug() << "[LoginScreen] Navigating after login to screen index:" << targetScreen;
            emit navigateTo(targetScreen);
        },
        [this](int statusCode, const QString &errorMessage) {
            qDebug() << "[LoginScreen] Login failure callback received. HTTP Status:" << statusCode
                     << "Message:" << errorMessage;
            m_loginBtn->setEnabled(true);
            m_loginBtn->setText("Sign In");
            if (statusCode == 401 || statusCode == 400) {
                m_errorLabel->setText(QString("❌ Invalid email or password. (HTTP %1)").arg(statusCode));
            } else if (statusCode == 0) {
                m_errorLabel->setText("❌ Unable to connect to backend server on 127.0.0.1:18080. Please ensure the backend is running.");
            } else {
                m_errorLabel->setText(QString("❌ %1 (HTTP %2)").arg(errorMessage).arg(statusCode));
            }
            m_errorLabel->show();
        }
    );
}