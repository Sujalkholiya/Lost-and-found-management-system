#include "StatCardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariantAnimation>
#include <QEasingCurve>

StatCardWidget::StatCardWidget(const QString& title, const QString& icon, ColorTheme theme, QWidget* parent)
    : QFrame(parent)
    , m_iconLabel(new QLabel(icon, this))
    , m_valueLabel(new QLabel("0", this))
    , m_titleLabel(new QLabel(title, this))
    , m_currentValue(0)
{
    setObjectName("statCard");
    setFrameShape(QFrame::StyledPanel);

    QString iconBgColor, iconTextColor;
    if (theme == Emerald) {
        iconBgColor = "#ECFDF5";
        iconTextColor = "#059669";
    } else if (theme == Rose) {
        iconBgColor = "#FFF1F2";
        iconTextColor = "#E11D48";
    } else if (theme == Amber) {
        iconBgColor = "#FFFBEB";
        iconTextColor = "#D97706";
    } else {
        iconBgColor = "#EEF2FF";
        iconTextColor = "#4F46E5";
    }

    m_iconLabel->setFixedSize(40, 40);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 18px; font-weight: bold; border-radius: 10px;"
    ).arg(iconBgColor, iconTextColor));

    m_valueLabel->setStyleSheet("font-size: 22px; font-weight: 800; color: #0F172A; background: transparent;");
    m_titleLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748B; background: transparent;");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 14, 16, 14);
    mainLayout->setSpacing(12);

    mainLayout->addWidget(m_iconLabel);

    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_valueLabel);

    mainLayout->addLayout(textLayout);
    mainLayout->addStretch();

    setStyleSheet(
        "StatCardWidget#statCard { "
        "  background-color: rgba(255, 255, 255, 0.92); "
        "  border: 1px solid #E2E8F0; "
        "  border-radius: 14px; "
        "}"
        "StatCardWidget#statCard:hover { "
        "  border: 1.5px solid #CBD5E1; "
        "  background-color: #FFFFFF; "
        "}"
    );
}

void StatCardWidget::setValue(int value)
{
    int startVal = m_currentValue;
    m_currentValue = value;

    if (startVal == value) {
        m_valueLabel->setText(QString::number(value));
        return;
    }

    // Smooth counting animation (350ms)
    QVariantAnimation *anim = new QVariantAnimation(this);
    anim->setDuration(350);
    anim->setStartValue(startVal);
    anim->setEndValue(value);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &val) {
        m_valueLabel->setText(QString::number(val.toInt()));
    });
    connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

void StatCardWidget::setValue(const QString& value)
{
    m_valueLabel->setText(value);
}