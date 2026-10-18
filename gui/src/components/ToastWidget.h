#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

enum class ToastType {
    Success,
    Error,
    Info,
    Warning
};

class ToastWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ToastWidget(QWidget *parent = nullptr);
    void showToast(const QString &message, ToastType type = ToastType::Info, int durationMs = 3500);

signals:
    void toastClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void hideAnimated();

private:
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
    QTimer *m_timer;
    QGraphicsOpacityEffect *m_opacityEffect;
    QParallelAnimationGroup *m_showAnimGroup;
    QParallelAnimationGroup *m_hideAnimGroup;
    QPropertyAnimation *m_posAnimIn;
    QPropertyAnimation *m_fadeAnimIn;
    QPropertyAnimation *m_posAnimOut;
    QPropertyAnimation *m_fadeAnimOut;
};

#endif // TOASTWIDGET_H