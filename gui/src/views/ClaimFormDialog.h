#ifndef CLAIMFORMDIALOG_H
#define CLAIMFORMDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include "../models/Item.h"
#include "../models/Claim.h"
#include "../network/ApiClient.h"

class ClaimFormDialog : public QDialog {
    Q_OBJECT

public:
    explicit ClaimFormDialog(const Item& item, ApiClient* apiClient, QWidget* parent = nullptr);

signals:
    void claimSubmitted(const Claim& claim);

private slots:
    void onSubmitClicked();
    void validateForm();

private:
    void setupUi();

    Item m_item;
    ApiClient* m_apiClient;

    QLineEdit* m_lastSeenEdit;
    QTextEdit* m_uniqueDetailsEdit;
    QTextEdit* m_contentsEdit;
    QLineEdit* m_additionalEdit;
    QCheckBox* m_affirmationCheckbox;

    QLabel* m_errorLabel;
    QPushButton* m_submitBtn;
    QPushButton* m_cancelBtn;
};

#endif // CLAIMFORMDIALOG_H

