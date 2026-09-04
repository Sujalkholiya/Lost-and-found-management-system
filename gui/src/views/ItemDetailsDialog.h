#ifndef ITEMDETAILSDIALOG_H
#define ITEMDETAILSDIALOG_H

#include <QDialog>
#include "../models/Item.h"
#include "../network/ApiClient.h"

class ItemDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItemDetailsDialog(const Item& item, QWidget* parent = nullptr);
    explicit ItemDetailsDialog(const Item& item, ApiClient* apiClient = nullptr, QWidget* parent = nullptr);

signals:
    void itemDeleted(int itemId);

private slots:
    void onDeleteClicked();

private:
    Item m_item;
    ApiClient* m_apiClient;
};

#endif // ITEMDETAILSDIALOG_H

