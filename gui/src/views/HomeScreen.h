#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "../network/ApiClient.h"
#include "../components/StatCardWidget.h"
#include "../models/Item.h"
#include "../components/ToastWidget.h"

class HomeScreen : public QWidget
{
    Q_OBJECT

public:
    explicit HomeScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void refreshData();

signals:
    void navigateTo(int screenIndex);
    void viewItemDetails(const Item &item);
    void showToast(const QString &message, ToastType type);

private slots:
    void updateAuthControls();
    void confirmDeleteItem(const Item &item);

private:
    void setupUI();
    void updateItems(const QVector<Item> &items);

    ApiClient *m_apiClient;

    QPushButton *m_authNavBtn;
    StatCardWidget *m_totalCard;
    StatCardWidget *m_lostCard;
    StatCardWidget *m_foundCard;
    QWidget *m_recentContainer;
    QGridLayout *m_recentGrid;
};

#endif // HOMESCREEN_H
