#ifndef ITEMDETAILSDIALOG_H
#define ITEMDETAILSDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include "../models/Item.h"
#include "../models/Claim.h"
#include "../network/ApiClient.h"

class ItemDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItemDetailsDialog(const Item& item, QWidget* parent = nullptr);
    explicit ItemDetailsDialog(const Item& item, ApiClient* apiClient = nullptr, QWidget* parent = nullptr);

signals:
    void itemDeleted(int itemId);
    void claimSubmitted(int itemId);

private slots:
    void onDeleteClicked();
    void onClaimClicked();
    void checkClaimStatus();

private:
    Item m_item;
    ApiClient* m_apiClient;
    Claim m_currentClaim;
    bool m_hasClaim;

    QPushButton* m_claimBtn;
    QFrame* m_claimStatusBox;
    QLabel* m_claimStatusLabel;
};

#endif // ITEMDETAILSDIALOG_H
