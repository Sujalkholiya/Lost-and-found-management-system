#ifndef DASHBOARDSCREEN_H
#define DASHBOARDSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QScrollArea>
#include "../network/ApiClient.h"
#include "../components/ToastWidget.h"
#include "../models/Item.h"
#include "../models/Claim.h"

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
    void profileClicked();

private slots:
    void confirmDeleteItem(const Item &item);

private:
    void setupUI();
    void updateProfile();
    void renderUserItems(const QVector<Item> &items);
    void renderUserClaims(const QVector<Claim> &claims);
    QString getGreeting() const;

    QFrame* createKpiCard(const QString &icon, const QString &label, const QString &supportingText, QLabel *valLabel, const QString &accentColor);
    QFrame* createQuickActionCard(const QString &icon, const QString &title, const QString &desc, const QString &btnText, const QString &btnStyle, int targetScreen);
    QWidget* createRecentReportRow(const Item &item);
    QLabel* createStatusBadge(const QString &text, const QString &textColor, const QString &bgColor);

    ApiClient *m_apiClient;

    // Header labels
    QLabel *m_greetingLabel;
    QLabel *m_subGreetingLabel;

    // Profile card widgets
    QLabel *m_profileAvatarLabel;
    QLabel *m_profileNameLabel;
    QLabel *m_profileRoleBadge;
    QLabel *m_profileAffiliationLabel;
    QLabel *m_profileIdLabel;
    QPushButton *m_changeProfileBtn;

    // KPI values
    QLabel *m_statTotalVal;
    QLabel *m_statLostVal;
    QLabel *m_statFoundVal;
    QLabel *m_statResolvedVal;

    // Claims section
    QFrame *m_claimsCard;
    QVBoxLayout *m_claimsCardLayout;

    // Recent reports section
    QLabel *m_reportsCountBadge;
    QFrame *m_reportsTableCard;
    QVBoxLayout *m_reportsRowsLayout;
};

#endif // DASHBOARDSCREEN_H