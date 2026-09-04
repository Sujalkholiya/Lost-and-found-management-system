#ifndef ITEMCARDWIDGET_H
#define ITEMCARDWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include "../models/Item.h"

class ItemCardWidget : public QFrame {
    Q_OBJECT

public:
    explicit ItemCardWidget(const Item& item, QWidget* parent = nullptr);
    Item item() const { return m_item; }

signals:
    void viewDetailsRequested(const Item& item);
    void deleteItemRequested(const Item& item);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    Item m_item;
    QLabel* m_typeBadge;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    QLabel* m_locationLabel;
    QLabel* m_userLabel;
    QLabel* m_imageThumbnail;
    QPushButton* m_detailsBtn;
    QPushButton* m_deleteBtn;
};

#endif // ITEMCARDWIDGET_H

