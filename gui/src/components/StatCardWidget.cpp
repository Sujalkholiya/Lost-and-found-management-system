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
        iconBgColor = "#EAFBF4";
        iconTextColor = "#16A66A";
    } else if (theme == Rose) {
        iconBgColor = "#FEF2F2";
        iconTextColor = "#EF4444";
    } else if (theme == Amber) {
        iconBgColor = "#FEF3C7";
        iconTextColor = "#F59E0B";
    } else {
        iconBgColor = "#EAF2FF";
        iconTextColor = "#2563EB";
    }

    m_iconLabel->setFixedSize(46, 46);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 20px; font-weight: bold; border-radius: 23px;"
    ).arg(iconBgColor, iconTextColor));

    m_valueLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: #172B4D; background: transparent;");
    m_titleLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #6B7FA3; background: transparent;");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(14);

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
        "  background-color: #FFFFFF; "
        "  border: 1px solid #E2EAF4; "
        "  border-radius: 18px; "
        "}"
        "StatCardWidget#statCard:hover { "
        "  border: 1px solid #CBD5E1; "
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