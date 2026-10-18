#ifndef ADMINSCREEN_H
#define ADMINSCREEN_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QTimer>
#include <QSet>
#include "../network/ApiClient.h"
#include "../components/StatCardWidget.h"
#include "../components/ToastWidget.h"
#include "../models/Item.h"
#include "../models/Claim.h"

class AdminScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AdminScreen(ApiClient *apiClient, QWidget *parent = nullptr);
    void refreshData();
    void refreshClaims();
    void checkPermissions();
    void switchToTab(int tabIndex);

signals:
    void navigateTo(int screenIndex);
    void viewItemDetails(const Item &item);
    void showToast(const QString &message, ToastType type);
    void tabChanged(int tabIndex);

public slots:
    void onClaimFilterBtnClicked(const QString& filter);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void applyTableFilter();
    void applyClaimsFilter();
    void pollPendingClaims();

private:
    void setupUI();
    void populateTable(const QVector<Item> &items);
    void populateClaimsTable(const QVector<Claim> &claims);
    void setLoading(bool loading);
    void showError(int statusCode, const QString &errorMessage);
    void openHandoverDialog(const Claim &claim);
    void showRejectionReasonDialog(const Claim &claim);

    ApiClient *m_apiClient;
    QVector<Item> m_adminItems;
    QVector<Claim> m_adminClaims;
    ClaimStats m_claimStats;
    QString m_currentClaimsFilter;

    // Header Badges
    QLabel *m_roleBadgeLabel;
    QLabel *m_pendingBadgeLabel;

    // Tabs
    QTabWidget *m_tabWidget;

    // Inventory Stat Cards
    StatCardWidget *m_totalCard;
    StatCardWidget *m_lostCard;
    StatCardWidget *m_foundCard;
    StatCardWidget *m_activeCard;

    // Claims KPI Cards (Pending Review, Approved, Handed Over, Rejected)
    StatCardWidget *m_pendingClaimsCard;
    StatCardWidget *m_approvedClaimsCard;
    StatCardWidget *m_handedOverClaimsCard;
    StatCardWidget *m_rejectedClaimsCard;

    // Filter and Search
    QLineEdit *m_searchEdit;
    QLineEdit *m_claimsSearchEdit;
    QPushButton *m_refreshBtn;
    QLabel *m_recordCountLabel;
    QLabel *m_claimsCountLabel;

    // Claims filter buttons
    QPushButton *m_cBtnAll;
    QPushButton *m_cBtnPending;
    QPushButton *m_cBtnApproved;
    QPushButton *m_cBtnHandedOver;
    QPushButton *m_cBtnRejected;

    // State Views
    QWidget *m_forbiddenBox;
    QWidget *m_contentBox;
    QWidget *m_loadingBox;
    QProgressBar *m_progressBar;
    QWidget *m_errorBox;
    QLabel *m_errorLabel;
    QPushButton *m_retryBtn;

    // Tables
    QTableWidget *m_table;
    QTableWidget *m_claimsTable;

    // Polling & Real-time Claim Monitoring
    QTimer *m_pollTimer;
    QSet<int> m_knownPendingClaimIds;
    bool m_isFirstPoll;
    bool m_isPolling;
};

#endif // ADMINSCREEN_H
