#ifndef EMPTYSTATEWIDGET_H
#define EMPTYSTATEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class EmptyStateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmptyStateWidget(const QString &icon, const QString &title, const QString &subtitle, const QString &buttonText = "", QWidget *parent = nullptr);
    void setMessage(const QString &title, const QString &subtitle);

signals:
    void actionClicked();

private:
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_subLabel;
    QPushButton *m_actionBtn;
};

#endif // EMPTYSTATEWIDGET_H
