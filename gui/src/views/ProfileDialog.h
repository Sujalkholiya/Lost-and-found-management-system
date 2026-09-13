#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "../network/ApiClient.h"
#include "../models/User.h"

class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(ApiClient *apiClient, QWidget *parent = nullptr);

signals:
    void profileUpdated(const QString &newName);

private slots:
    void onSaveClicked();

private:
    void setupUI();
    void updateAvatar(const QString &name);

    ApiClient *m_apiClient;
    QLabel *m_avatarLabel;
    QLabel *m_roleBadge;
    QLabel *m_userIdLabel;
    QLineEdit *m_nameEdit;
    QLineEdit *m_emailEdit;
    QLabel *m_errorLabel;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // PROFILEDIALOG_H

