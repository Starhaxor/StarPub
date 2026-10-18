#pragma once

#include "api/KeyStarClient.h"
#include "models/Models.h"

#include <QObject>
#include <functional>

namespace KeyStar {

// Spec Section 17 — Separation of concerns: UI -> Service -> KeyStarClient.
// Services expose async callbacks; while no endpoint is configured they
// serve MockStore with a short simulated latency (keeps UI async patterns
// honest). When an endpoint IS configured, list/stats still merge local mock
// until the real API contract lands — mutations always log to audit trail.

struct LicenseFilter {
    QString text;    // matches key / user / hwid / product / status
    QString status = "ALL";
    QString product = "ALL";
    int page = 0;
    int pageSize = 50;
};

struct LicensePage {
    QVector<License> rows;
    int total = 0;
};

class LicenseService : public QObject {
    Q_OBJECT
public:
    explicit LicenseService(KeyStarClient *client, QObject *parent = nullptr);

    using PageCb = std::function<void(const LicensePage &)>;
    using LicenseCb = std::function<void(bool ok, const License &, const QString &msg)>;
    using KeysCb = std::function<void(bool ok, const QStringList &, const QString &msg)>;
    using BoolCb = std::function<void(bool ok, const QString &msg)>;

    void list(const LicenseFilter &f, PageCb cb);
    void create(const QString &product, int days, int qty, const QString &notes, KeysCb cb);
    void extend(const QString &key, int days, LicenseCb cb);
    void resetHwid(const QString &key, BoolCb cb);
    void revoke(const QString &key, BoolCb cb);
    void remove(const QString &key, BoolCb cb);

private:
    KeyStarClient *m_client;
};

class ProductService : public QObject {
    Q_OBJECT
public:
    explicit ProductService(KeyStarClient *client, QObject *parent = nullptr);
    using ProductsCb = std::function<void(const QVector<Product> &)>;
    void list(ProductsCb cb);

private:
    KeyStarClient *m_client;
};

class SystemService : public QObject {
    Q_OBJECT
public:
    explicit SystemService(KeyStarClient *client, QObject *parent = nullptr);
    using StatsCb = std::function<void(const DashboardStats &)>;
    using LogsCb = std::function<void(const QVector<LogEntry> &)>;
    using UsersCb = std::function<void(const QVector<User> &)>;
    using DevicesCb = std::function<void(const QVector<Device> &)>;
    using SessionsCb = std::function<void(const QVector<Session> &)>;

    void dashboard(StatsCb cb);
    void recentActivity(int limit, LogsCb cb);
    void logs(LogsCb cb);
    void users(UsersCb cb);
    void devices(DevicesCb cb);
    void sessions(SessionsCb cb);
    void ping(std::function<void(bool online, qint64 ms)> cb);

private:
    KeyStarClient *m_client;
};

} // namespace KeyStar
