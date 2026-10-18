#include "services/Services.h"

#include "core/Config.h"
#include "services/MockStore.h"

#include <QElapsedTimer>
#include <QTimer>

namespace KeyStar {

static void async(QObject *ctx, std::function<void()> fn) {
    // Simulated network latency (~120ms) so loading states are exercised.
    QTimer::singleShot(120, ctx, std::move(fn));
}

static bool matches(const License &l, const LicenseFilter &f) {
    if (f.status != "ALL" && l.status != f.status)
        return false;
    if (f.product != "ALL" && l.product != f.product)
        return false;
    if (!f.text.isEmpty()) {
        const QString t = f.text.toLower();
        if (!l.key.toLower().contains(t) && !l.user.toLower().contains(t)
            && !l.hwid.toLower().contains(t) && !l.product.toLower().contains(t)
            && !l.status.toLower().contains(t))
            return false;
    }
    return true;
}

LicenseService::LicenseService(KeyStarClient *client, QObject *parent)
    : QObject(parent), m_client(client) {}

void LicenseService::list(const LicenseFilter &f, PageCb cb) {
    async(this, [f, cb]() {
        LicensePage p;
        for (const auto &l : MockStore::instance().licenses)
            if (matches(l, f))
                p.rows.push_back(l);
        p.total = p.rows.size();
        const int from = qMin(f.page * f.pageSize, p.total);
        p.rows = p.rows.mid(from, f.pageSize);
        cb(p);
    });
}

void LicenseService::create(const QString &product, int days, int qty,
                            const QString &notes, KeysCb cb) {
    async(this, [=]() {
        QStringList keys;
        auto &store = MockStore::instance();
        const QDateTime now = QDateTime::currentDateTime();
        for (int i = 0; i < qBound(1, qty, 100); ++i) {
            License l;
            l.key = QString("%1-%2-%3-%4")
                        .arg(QString::number(rand() % 0xFFFF, 16).toUpper().rightJustified(4, '0'),
                             QString::number(rand() % 0xFFFF, 16).toUpper().rightJustified(4, '0'),
                             QString::number(rand() % 0xFFFF, 16).toUpper().rightJustified(4, '0'),
                             QString::number(rand() % 0xFFFF, 16).toUpper().rightJustified(4, '0'));
            l.product = product;
            l.status = "ACTIVE";
            l.created = now;
            l.expires = now.addDays(days);
            l.notes = notes;
            l.devicesMax = 2;
            l.lastAuth = now;
            store.licenses.prepend(l);
            keys << l.key;
        }
        store.addLog("LICENSE_CREATED", "admin", keys.first().left(14) + "…");
        cb(true, keys, "License created successfully");
    });
}

void LicenseService::extend(const QString &key, int days, LicenseCb cb) {
    async(this, [=]() {
        for (auto &l : MockStore::instance().licenses) {
            if (l.key == key) {
                QDateTime base = l.expires.isValid() ? l.expires : QDateTime::currentDateTime();
                l.expires = base.addDays(days);
                if (l.status == "EXPIRED")
                    l.status = "ACTIVE";
                MockStore::instance().addLog("LICENSE_EXTENDED", "admin", key.left(14) + "…");
                cb(true, l, "License extended");
                return;
            }
        }
        cb(false, License(), "License not found");
    });
}

void LicenseService::resetHwid(const QString &key, BoolCb cb) {
    async(this, [=]() {
        for (auto &l : MockStore::instance().licenses) {
            if (l.key == key) {
                l.hwid.clear();
                l.devicesUsed = 0;
                l.lastAuth = QDateTime();
                MockStore::instance().addLog("HWID_RESET", "admin", l.user, "");
                cb(true, "HWID reset");
                return;
            }
        }
        cb(false, "License not found");
    });
}

void LicenseService::revoke(const QString &key, BoolCb cb) {
    async(this, [=]() {
        for (auto &l : MockStore::instance().licenses) {
            if (l.key == key) {
                l.status = "REVOKED";
                MockStore::instance().addLog("LICENSE_REVOKED", "admin", key.left(14) + "…");
                cb(true, "License revoked");
                return;
            }
        }
        cb(false, "License not found");
    });
}

void LicenseService::remove(const QString &key, BoolCb cb) {
    async(this, [=]() {
        auto &v = MockStore::instance().licenses;
        for (int i = 0; i < v.size(); ++i) {
            if (v[i].key == key) {
                v.removeAt(i);
                MockStore::instance().addLog("LICENSE_DELETED", "admin", key.left(14) + "…");
                cb(true, "License deleted");
                return;
            }
        }
        cb(false, "License not found");
    });
}

ProductService::ProductService(KeyStarClient *client, QObject *parent)
    : QObject(parent), m_client(client) {}

void ProductService::list(ProductsCb cb) {
    async(this, [cb]() { cb(MockStore::instance().products); });
}

SystemService::SystemService(KeyStarClient *client, QObject *parent)
    : QObject(parent), m_client(client) {}

void SystemService::dashboard(StatsCb cb) {
    async(this, [cb]() {
        DashboardStats s;
        const auto &ls = MockStore::instance().licenses;
        const QDateTime today = QDateTime::currentDateTime();
        for (const auto &l : ls) {
            if (l.status == "ACTIVE")
                ++s.activeLicenses;
            if (l.status == "EXPIRED")
                ++s.expiredLicenses;
            if (l.created.date() == today.date())
                ++s.createdToday;
        }
        s.totalUsers = MockStore::instance().users.size();
        s.registeredDevices = MockStore::instance().devices.size();
        s.activeSessions = std::count_if(
            MockStore::instance().sessions.begin(), MockStore::instance().sessions.end(),
            [](const Session &x) { return x.status == "ACTIVE"; });
        cb(s);
    });
}

void SystemService::recentActivity(int limit, LogsCb cb) {
    async(this, [limit, cb]() { cb(MockStore::instance().logs.mid(0, limit)); });
}

void SystemService::logs(LogsCb cb) {
    async(this, [cb]() { cb(MockStore::instance().logs); });
}

void SystemService::users(UsersCb cb) {
    async(this, [cb]() { cb(MockStore::instance().users); });
}

void SystemService::devices(DevicesCb cb) {
    async(this, [cb]() { cb(MockStore::instance().devices); });
}

void SystemService::sessions(SessionsCb cb) {
    async(this, [cb]() { cb(MockStore::instance().sessions); });
}

void SystemService::ping(std::function<void(bool, qint64)> cb) {
    if (!m_client->isConfigured()) {
        async(this, [cb]() { cb(true, 3); }); // local/mock mode counts as ready
        return;
    }
    QElapsedTimer t;
    t.start();
    m_client->ping([cb, t](const ApiResult &r) mutable {
        cb(r.ok, t.elapsed());
    });
}

} // namespace KeyStar
