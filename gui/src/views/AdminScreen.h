#ifndef ADMINSCREEN_H
#define ADMINSCREEN_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "../network/ApiClient.h"
#include "../components/StatCardWidget.h"
#include "../components/ToastWidget.h"
#include "../models/Item.h"

class AdminScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AdminScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void refreshData();
    void checkPermissions();

signals:
    void navigateTo(int screenIndex);
    void viewItemDetails(const Item &item);
    void showToast(const QString &message, ToastType type);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void applyTableFilter();

private:
    void setupUI();
    void populateTable(const QVector<Item> &items);
    void setLoading(bool loading);
    void showError(int statusCode, const QString &errorMessage);

    ApiClient *m_apiClient;
    QVector<Item> m_adminItems;

    // Stat Cards
    StatCardWidget *m_totalCard;
    StatCardWidget *m_lostCard;
    StatCardWidget *m_foundCard;
    StatCardWidget *m_activeCard;

    // Filter and Search
    QLineEdit *m_searchEdit;
    QPushButton *m_refreshBtn;
    QLabel *m_recordCountLabel;

    // State Views
    QWidget *m_forbiddenBox;
    QWidget *m_contentBox;
    QWidget *m_loadingBox;
    QProgressBar *m_progressBar;
    QWidget *m_errorBox;
    QLabel *m_errorLabel;
    QPushButton *m_retryBtn;

    // Table
    QTableWidget *m_table;
};

#endif // ADMINSCREEN_H
