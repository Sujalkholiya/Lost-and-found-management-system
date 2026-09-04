#ifndef ACCESSDENIEDSCREEN_H
#define ACCESSDENIEDSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class AccessDeniedScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AccessDeniedScreen(QWidget *parent = nullptr);

signals:
    void homeRequested();
    void loginRequested();

private:
    void setupUI();
};

#endif // ACCESSDENIEDSCREEN_H
