#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVector>
#include <QWidget>

namespace KeyStar {
class KeyStarClient;
class LicenseService;
class ProductService;
class SystemService;
}

namespace KeyStar::Ui {
class DashboardPage;
class LicensesPage;
class ProductsPage;
class UsersPage;
class DevicesPage;
class SessionsPage;
class LogsPage;
class SettingsPage;
class LoadingOverlay;
}

namespace KeyStar::Ui {

// Spec Section 5/6/23 — Main window: fixed sidebar (230px), top bar with
// connection indicator, stacked content, bottom status bar.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(KeyStarClient *client, LicenseService *lics,
                        ProductService *prods, SystemService *sys,
                        QWidget *parent = nullptr);

public slots:
    void goTo(const QString &name);

protected:
    void showEvent(QShowEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private slots:
    void showToast(const QString &msg, int kind);
    void setLoading(bool on);
    void updateConnection();
    void refreshCurrent();

private:
    void buildSidebar();
    void buildTopBar();
    QPushButton *sideItem(const QString &name, const QString &icon);

    KeyStarClient *m_client;
    LicenseService *m_lics;
    SystemService *m_sys;

    QWidget *m_root;
    QVector<QPushButton *> m_nav;
    QStackedWidget *m_stack;
    QLabel *m_connText;
    QLabel *m_apiStatus;
    QLabel *m_versionLabel;
    QWidget *m_searchPill = nullptr;
    LoadingOverlay *m_overlay = nullptr;

    DashboardPage *m_dash;
    LicensesPage *m_licenses;
    ProductsPage *m_products;
    UsersPage *m_users;
    DevicesPage *m_devices;
    SessionsPage *m_sessions;
    LogsPage *m_logs;
    SettingsPage *m_settings;

    QStringList m_order; // stack index -> name
    QTimer *m_pollTimer = nullptr;
};

} // namespace KeyStar::Ui
