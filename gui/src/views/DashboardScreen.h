#ifndef DASHBOARDSCREEN_H
#define DASHBOARDSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include "../network/ApiClient.h"
#include "../components/ToastWidget.h"
#include "../models/Item.h"

class DashboardScreen : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void refreshData();

signals:
    void navigateTo(int screenIndex);
    void viewItemDetails(const Item &item);
    void showToast(const QString &message, ToastType type);

private slots:
    void confirmDeleteItem(const Item &item);

private:
    void updateProfile();
    void renderUserItems(const QVector<Item> &items);

    ApiClient *m_apiClient;

    QLabel *m_greetingLabel;
    QLabel *m_emailLabel;
    QLabel *m_roleBadge;
    QLabel *m_statTotalVal;
    QLabel *m_statLostVal;
    QLabel *m_statFoundVal;
    QLabel *m_itemCountLabel;
    QWidget *m_itemsContainer;
    QGridLayout *m_itemsGrid;
};

#endif // DASHBOARDSCREEN_H