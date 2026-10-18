#ifndef ADMINCLAIMREVIEWDIALOG_H
#define ADMINCLAIMREVIEWDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include "../models/Claim.h"
#include "../network/ApiClient.h"

class AdminClaimReviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit AdminClaimReviewDialog(const Claim& claim, ApiClient* apiClient, QWidget* parent = nullptr);

signals:
    void claimUpdated();
    void claimApproved(const QString& reference);
    void claimRejected(const QString& reference);
    void claimHandedOver(const QString& reference);

private slots:
    void onApproveClicked();
    void onRejectClicked();
    void onHandoverClicked();

private:
    void setupUi();

    Claim m_claim;
    ApiClient* m_apiClient;

    QPushButton* m_approveBtn;
    QPushButton* m_rejectBtn;
    QPushButton* m_handoverBtn;
    QCheckBox* m_handoverConfirmCheck;
    QLineEdit* m_pickupLocationEdit;
};

#endif // ADMINCLAIMREVIEWDIALOG_H

