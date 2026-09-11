#include "AccessDeniedScreen.h"
#include <QHBoxLayout>
#include <QFrame>

AccessDeniedScreen::AccessDeniedScreen(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AccessDeniedScreen::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 60, 40, 60);
    mainLayout->setAlignment(Qt::AlignCenter);

    QFrame *card = new QFrame(this);
    card->setFixedWidth(500);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 16px;"
        "   padding: 36px;"
        "}"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(16);
    cardLayout->setAlignment(Qt::AlignCenter);

    QLabel *shieldIcon = new QLabel("🔒", card);
    shieldIcon->setStyleSheet("font-size: 56px; background: transparent;");
    shieldIcon->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(shieldIcon);

    QLabel *titleLabel = new QLabel("Access Restricted", card);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #DC2626; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    QLabel *descLabel = new QLabel(
        "You do not have administrative privileges to view this section.\n"
        "Please contact the campus system administrator or sign in with an authorized account.",
        card
    );
    descLabel->setStyleSheet("font-size: 14px; color: #64748B; background: transparent;");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(descLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    QPushButton *homeBtn = new QPushButton("🏠 Return Home", card);
    homeBtn->setCursor(Qt::PointingHandCursor);
    homeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   font-weight: 600;"
        "   padding: 10px 18px;"
        "   border-radius: 6px;"
        "   border: 1px solid #CBD5E1;"
        "}"
        "QPushButton:hover { background-color: #E2E8F0; }"
    );
    connect(homeBtn, &QPushButton::clicked, this, &AccessDeniedScreen::homeRequested);

    QPushButton *loginBtn = new QPushButton("🔑 Admin Sign In", card);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4F46E5;"
        "   color: #FFFFFF;"
        "   font-weight: 600;"
        "   padding: 10px 18px;"
        "   border-radius: 6px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(loginBtn, &QPushButton::clicked, this, &AccessDeniedScreen::loginRequested);

    btnLayout->addWidget(homeBtn);
    btnLayout->addWidget(loginBtn);
    cardLayout->addLayout(btnLayout);

    mainLayout->addWidget(card);
}
