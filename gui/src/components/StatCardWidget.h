#ifndef STATCARDWIDGET_H
#define STATCARDWIDGET_H

#include <QFrame>
#include <QLabel>

class StatCardWidget : public QFrame
{
    Q_OBJECT

public:
    enum ColorTheme {
        Indigo,
        Emerald,
        Rose,
        Amber
    };

    explicit StatCardWidget(const QString& title, const QString& icon, ColorTheme theme = Indigo, QWidget* parent = nullptr);
    void setValue(int value);
    void setValue(const QString& value);

private:
    QLabel* m_iconLabel;
    QLabel* m_valueLabel;
    QLabel* m_titleLabel;
    int m_currentValue;
};

#endif // STATCARDWIDGET_H