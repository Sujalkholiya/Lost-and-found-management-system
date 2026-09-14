#include "EmptyStateWidget.h"
#include "Theme.h"
#include <QPixmap>

EmptyStateWidget::EmptyStateWidget(const QString &icon, const QString &title, const QString &subtitle, const QString &buttonText, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 40, 32, 40);
    layout->setSpacing(12);
    layout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("background: transparent;");

    // Safe loading of local visual asset
    QPixmap emptyPix(":/assets/resources/images/empty_search.png");
    if (!emptyPix.isNull()) {
        m_iconLabel->setPixmap(emptyPix.scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_iconLabel->setText(icon);
        m_iconLabel->setStyleSheet("font-size: 48px; background: transparent;");
    }
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1; background: transparent;").arg(Theme::Text::Primary));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_titleLabel);

    m_subLabel = new QLabel(subtitle, this);
    m_subLabel->setStyleSheet(QString("font-size: 14px; color: %1; background: transparent; max-width: 420px;").arg(Theme::Text::Secondary));
    m_subLabel->setWordWrap(true);
    m_subLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_subLabel);

    if (!buttonText.isEmpty()) {
        m_actionBtn = new QPushButton(buttonText, this);
        m_actionBtn->setCursor(Qt::PointingHandCursor);
        m_actionBtn->setStyleSheet(Theme::primaryButton(8, 22, 10, 13));
        connect(m_actionBtn, &QPushButton::clicked, this, &EmptyStateWidget::actionClicked);
        layout->addWidget(m_actionBtn);
    }
}

void EmptyStateWidget::setMessage(const QString &title, const QString &subtitle)
{
    m_titleLabel->setText(title);
    m_subLabel->setText(subtitle);
}