#pragma once

#include <QGridLayout>
#include <QVector>
#include <QWidget>

class QLabel;
class QResizeEvent;

namespace KeyStar {
class SystemService;
}

namespace KeyStar::Ui {

// Spec Section 7 — Dashboard: only useful info. Metric cards + recent activity.
class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(SystemService *sys, QWidget *parent = nullptr);
    void refresh();

signals:
    void toast(const QString &msg, int kind); // 0 ok, 1 warn, 2 err
    void loading(bool on);
    void goLogs();
    void goSettings();

private:
    void layoutMetrics();
    void resizeEvent(QResizeEvent *e) override;

    SystemService *m_sys;
    class MetricCard *m_active;
    class MetricCard *m_expired;
    class MetricCard *m_users;
    class MetricCard *m_sessions;
    class MetricCard *m_devices;
    QWidget *m_activityBox;
    class StatusBadge *m_apiBadge;
    QLabel *m_prodLabel;
    QGridLayout *m_grid = nullptr;
    QVector<class MetricCard *> m_cards;
    int m_cols = 0;
};

} // namespace KeyStar::Ui
