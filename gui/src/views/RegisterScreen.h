#ifndef REGISTERSCREEN_H
#define REGISTERSCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../network/ApiClient.h"
#include "../components/ToastWidget.h"

class RegisterScreen : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void resetForm();

signals:
    void navigateTo(int screenIndex);
    void showToast(const QString &message, ToastType type);

private slots:
    void handleRegister();
    void togglePasswordVisibility();

private:
    ApiClient *m_apiClient;

    QLineEdit *m_nameEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmPassEdit;
    QLineEdit *m_phoneEdit;
    QPushButton *m_showPassBtn;
    QPushButton *m_registerBtn;
    QPushButton *m_loginLink;
    QPushButton *m_backBtn;
    QLabel *m_errorLabel;
};

#endif // REGISTERSCREEN_H