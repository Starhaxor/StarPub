#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

namespace KeyStar {
class SystemService;
struct AppConfig;
struct LogEntry;
}

namespace KeyStar::Ui {

// Spec 12/13/14 — Phase 2 read-only tables (Users / Devices / Sessions).
class UsersPage : public QWidget {
    Q_OBJECT
public:
    explicit UsersPage(SystemService *sys, QWidget *parent = nullptr);
    void refresh();
signals:
    void loading(bool on);

private:
    SystemService *m_sys;
    QTableWidget *m_table;
};

class DevicesPage : public QWidget {
    Q_OBJECT
public:
    explicit DevicesPage(SystemService *sys, QWidget *parent = nullptr);
    void refresh();
signals:
    void loading(bool on);

private:
    SystemService *m_sys;
    QTableWidget *m_table;
};

class SessionsPage : public QWidget {
    Q_OBJECT
public:
    explicit SessionsPage(SystemService *sys, QWidget *parent = nullptr);
    void refresh();
signals:
    void loading(bool on);

private:
    SystemService *m_sys;
    QTableWidget *m_table;
};

// Spec Section 15 — Audit logs (read-only) + detail panel.
class LogsPage : public QWidget {
    Q_OBJECT
public:
    explicit LogsPage(SystemService *sys, QWidget *parent = nullptr);
    void refresh();
signals:
    void loading(bool on);

private:
    void fillTable();
    SystemService *m_sys;
    QTableWidget *m_table;
    QWidget *m_detail;
    class SearchBar *m_search;
    QComboBox *m_resultBox;
    QVector<KeyStar::LogEntry> m_all;
    QVector<KeyStar::LogEntry> m_shown;
    void showDetail(const KeyStar::LogEntry &e);
};

// Spec Section 31/37 — Settings: General / API / Security / Developer.
class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(SystemService *sys, QWidget *parent = nullptr);
signals:
    void toast(const QString &msg, int kind);
    void connectionChanged();

private:
    void save();
    SystemService *m_sys;
    QLineEdit *m_endpoint;
    QSpinBox *m_timeout;
    QComboBox *m_startup;
    QCheckBox *m_notif;
    QCheckBox *m_polling;
    QSpinBox *m_pollSec;
    QSpinBox *m_sessionMin;
    QCheckBox *m_devMode;
    QCheckBox *m_debugLog;
    QLabel *m_connLabel;
};

} // namespace KeyStar::Ui
