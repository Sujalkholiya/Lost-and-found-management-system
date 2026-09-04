#ifndef BROWSESCREEN_H
#define BROWSESCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QProgressBar>
#include "../network/ApiClient.h"
#include "../models/Item.h"
#include "../components/ToastWidget.h"

class BrowseScreen : public QWidget
{
    Q_OBJECT

public:
    explicit BrowseScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void refreshData();

signals:
    void navigateTo(int screenIndex);
    void viewItemDetails(const Item &item);
    void showToast(const QString &message, ToastType type);

private slots:
    void applyFilters();
    void onFilterButtonClicked(const QString &filter);
    void resetFilters();
    void confirmDeleteItem(const Item &item);

private:
    void setupUI();
    void renderItems(const QVector<Item> &items);
    void showLoading(bool loading);
    void showError(const QString &errorMessage);

    ApiClient *m_apiClient;
    QVector<Item> m_allItems;
    QString m_selectedTypeFilter;

    QLineEdit *m_searchEdit;
    QPushButton *m_allBtn;
    QPushButton *m_lostBtn;
    QPushButton *m_foundBtn;
    QComboBox *m_locationCombo;
    QPushButton *m_resetBtn;
    QLabel *m_countLabel;

    QWidget *m_loadingBox;
    QLabel *m_loadingLabel;
    QProgressBar *m_progressBar;

    QWidget *m_errorBox;
    QLabel *m_errorLabel;
    QPushButton *m_retryBtn;

    QWidget *m_gridContainer;
    QGridLayout *m_cardsGrid;
};

#endif // BROWSESCREEN_H