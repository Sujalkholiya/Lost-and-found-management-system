#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../network/ApiClient.h"
#include "../components/ToastWidget.h"

class LoginScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoginScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void resetForm();

signals:
    void navigateTo(int screenIndex);
    void showToast(const QString &message, ToastType type);

private slots:
    void handleLogin();
    void togglePasswordVisibility();

private:
    ApiClient *m_apiClient;

    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_showPassBtn;
    QPushButton *m_loginBtn;
    QPushButton *m_registerLink;
    QPushButton *m_backBtn;
    QLabel *m_errorLabel;
};

#endif // LOGINSCREEN_H