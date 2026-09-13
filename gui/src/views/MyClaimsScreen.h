#ifndef MYCLAIMSSCREEN_H
#define MYCLAIMSSCREEN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVector>
#include "../network/ApiClient.h"
#include "../models/Claim.h"
#include "../components/ToastWidget.h"

class MyClaimsScreen : public QWidget {
    Q_OBJECT

public:
    explicit MyClaimsScreen(ApiClient* apiClient, QWidget* parent = nullptr);
    void refreshData();

signals:
    void navigateTo(int screenIndex);
    void showToast(const QString &message, ToastType type);

private slots:
    void onFilterClicked(const QString& filter);
    void loadClaims();

private:
    void setupUi();
    void renderClaims();
    QWidget* createClaimCard(const Claim& claim);

    ApiClient* m_apiClient;
    QVector<Claim> m_allClaims;
    QString m_currentFilter; // "all", "pending", "approved", "handed_over", "rejected"

    QPushButton* m_btnAll;
    QPushButton* m_btnPending;
    QPushButton* m_btnApproved;
    QPushButton* m_btnHandedOver;
    QPushButton* m_btnRejected;
    QPushButton* m_refreshBtn;

    QLabel* m_countLabel;
    QWidget* m_cardsContainer;
    QVBoxLayout* m_cardsLayout;
    QWidget* m_loadingWidget;
    QWidget* m_emptyWidget;
};

#endif // MYCLAIMSSCREEN_H

