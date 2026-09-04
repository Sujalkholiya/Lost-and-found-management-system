#ifndef REPORTSCREEN_H
#define REPORTSCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "../network/ApiClient.h"
#include "../components/ToastWidget.h"

class ReportScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ReportScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void resetForm();
    void setReportType(const QString &type);
    void checkAuthentication();

signals:
    void navigateTo(int screenIndex);
    void showToast(const QString &message, ToastType type);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void submitReport();
    void chooseImage();
    void removeImage();

private:
    void setupUI();
    void setLoading(bool loading);
    void showValidationError(const QString &error);

    ApiClient *m_apiClient;

    QWidget *m_authRequiredCard;
    QWidget *m_formCard;

    QLineEdit *m_titleEdit;
    QComboBox *m_typeCombo;
    QComboBox *m_locationCombo;
    QLineEdit *m_customLocationEdit;
    QTextEdit *m_descEdit;

    // Image attachment
    QString m_selectedImagePath;
    QWidget *m_imagePreviewContainer;
    QLabel *m_imagePreviewLabel;
    QLabel *m_imageFileNameLabel;
    QPushButton *m_attachImageBtn;
    QPushButton *m_removeImageBtn;

    QPushButton *m_submitBtn;
    QPushButton *m_cancelBtn;

    QLabel *m_errorLabel;
    QProgressBar *m_progressBar;
};

#endif // REPORTSCREEN_H
