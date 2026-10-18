#include "ToastWidget.h"
#include <QHBoxLayout>

ToastWidget::ToastWidget(QWidget *parent)
    : QWidget(parent)
    , m_iconLabel(new QLabel(this))
    , m_textLabel(new QLabel(this))
    , m_timer(new QTimer(this))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
    , m_showAnimGroup(new QParallelAnimationGroup(this))
    , m_hideAnimGroup(new QParallelAnimationGroup(this))
    , m_posAnimIn(new QPropertyAnimation(this, "pos", this))
    , m_fadeAnimIn(new QPropertyAnimation(m_opacityEffect, "opacity", this))
    , m_posAnimOut(new QPropertyAnimation(this, "pos", this))
    , m_fadeAnimOut(new QPropertyAnimation(m_opacityEffect, "opacity", this))
{
    setCursor(Qt::PointingHandCursor);
    setGraphicsEffect(m_opacityEffect);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 18, 10);
    layout->setSpacing(10);

    m_iconLabel->setStyleSheet("font-size: 16px; background: transparent;");
    m_textLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #FFFFFF; background: transparent;");

    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);

    // Setup Inward Animations
    m_posAnimIn->setDuration(220);
    m_posAnimIn->setEasingCurve(QEasingCurve::OutCubic);

    m_fadeAnimIn->setDuration(200);
    m_fadeAnimIn->setStartValue(0.0);
    m_fadeAnimIn->setEndValue(1.0);
    m_fadeAnimIn->setEasingCurve(QEasingCurve::OutQuad);

    m_showAnimGroup->addAnimation(m_posAnimIn);
    m_showAnimGroup->addAnimation(m_fadeAnimIn);

    // Setup Outward Animations
    m_posAnimOut->setDuration(180);
    m_posAnimOut->setEasingCurve(QEasingCurve::InCubic);

    m_fadeAnimOut->setDuration(180);
    m_fadeAnimOut->setStartValue(1.0);
    m_fadeAnimOut->setEndValue(0.0);
    m_fadeAnimOut->setEasingCurve(QEasingCurve::InQuad);

    m_hideAnimGroup->addAnimation(m_posAnimOut);
    m_hideAnimGroup->addAnimation(m_fadeAnimOut);

    connect(m_hideAnimGroup, &QParallelAnimationGroup::finished, this, &QWidget::hide);

    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ToastWidget::hideAnimated);

    hide();
}

void ToastWidget::showToast(const QString &message, ToastType type, int durationMs)
{
    m_timer->stop();
    m_hideAnimGroup->stop();

    QString bgColor;
    QString icon;

    switch (type) {
        case ToastType::Success:
            bgColor = "#059669";
            icon = "✅";
            break;
        case ToastType::Error:
            bgColor = "#DC2626";
            icon = "❌";
            break;
        case ToastType::Warning:
            bgColor = "#D97706";
            icon = "⚠️";
            break;
        case ToastType::Info:
        default:
            bgColor = "#2563EB";
            icon = "ℹ️";
            break;
    }

    setStyleSheet(QString(
        "ToastWidget {"
        "   background-color: %1;"
        "   border-radius: 8px;"
        "   border: 1px solid rgba(255, 255, 255, 0.3);"
        "}"
    ).arg(bgColor));

    m_iconLabel->setText(icon);
    m_textLabel->setText(message);
    adjustSize();

    int targetX = 0;
    if (parentWidget()) {
        targetX = (parentWidget()->width() - width()) / 2;
    }

    QPoint startPos(targetX, 54);
    QPoint endPos(targetX, 74);

    move(startPos);
    m_opacityEffect->setOpacity(0.0);

    m_posAnimIn->setStartValue(startPos);
    m_posAnimIn->setEndValue(endPos);

    show();
    raise();

    m_showAnimGroup->stop();
    m_showAnimGroup->start();

    m_timer->start(durationMs);
}

void ToastWidget::hideAnimated()
{
    if (!isVisible()) return;

    m_showAnimGroup->stop();

    QPoint currentPos = pos();
    QPoint endPos(currentPos.x(), currentPos.y() - 16);

    m_posAnimOut->setStartValue(currentPos);
    m_posAnimOut->setEndValue(endPos);

    m_hideAnimGroup->start();
}

void ToastWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit toastClicked();
    hideAnimated();
}