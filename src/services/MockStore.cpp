#include "services/MockStore.h"

namespace KeyStar {

MockStore &MockStore::instance() {
    static MockStore s;
    return s;
}

void MockStore::addLog(const QString &action, const QString &actor,
                       const QString &target, const QString &device,
                       const QString &result) {
    LogEntry e;
    e.time = QDateTime::currentDateTime();
    e.action = action;
    e.actor = actor;
    e.target = target;
    e.device = device;
    e.result = result;
    logs.prepend(e);
}

static QString genKey() {
    const char *hex = "0123456789ABCDEF";
    auto grp = [&]() {
        QString g;
        for (int i = 0; i < 4; ++i)
            g += hex[rand() % 16];
        return g;
    };
    return QString("%1-%2-%3-%4").arg(grp(), grp(), grp(), grp());
}

void MockStore::seed() {
    if (!licenses.isEmpty())
        return;
    srand(4242);
    const QDateTime now = QDateTime::currentDateTime();

    products = {
        {"StarLoader", "starloader", "2.4.1", "ACTIVE", 0, now.addDays(-220)},
        {"Neuronot", "neuronot", "1.2.0", "ACTIVE", 0, now.addDays(-140)},
        {"Pro", "pro", "3.0.0", "ACTIVE", 0, now.addDays(-90)},
    };

    users = {
        {"admin", 4, 2, "ACTIVE", now.addSecs(-3600)},
        {"user_1029", 2, 3, "ACTIVE", now.addSecs(-1260)},
        {"user_2041", 2, 3, "REVOKED", now.addDays(-2)},
        {"user_3110", 1, 1, "ACTIVE", now.addDays(-1)},
    };

    const QStringList sts = {"ACTIVE", "ACTIVE", "ACTIVE", "ACTIVE", "EXPIRED", "REVOKED"};
    for (int i = 0; i < 42; ++i) {
        License l;
        l.key = genKey();
        l.product = (i % 4 == 3) ? "Neuronot" : (i % 9 == 8 ? "Pro" : "StarLoader");
        l.status = sts[i % sts.size()];
        l.created = now.addDays(-(i * 3 + 1));
        l.expires = (l.status == "REVOKED") ? QDateTime() : l.created.addDays(30);
        l.user = users[i % users.size()].username;
        l.hwid = QString("A8F2-91C4-%1").arg(1000 + i);
        l.devicesUsed = 1;
        l.devicesMax = 2;
        l.lastAuth = now.addSecs(-(i * 613));
        licenses.push_back(l);
    }
    // Pin the spec's example row on top for familiarity
    licenses[0].key = "XXXX-7A92-19FD-82BC";
    licenses[0].product = "StarLoader";
    licenses[0].status = "ACTIVE";

    for (auto &p : products)
        p.licenseCount = std::count_if(licenses.begin(), licenses.end(),
            [&](const License &l) { return l.product == p.name; });

    devices = {
        {"A8F2-91C4-72DE", "user_1029", "StarLoader", now.addDays(-4), now.addSecs(-60), "ACTIVE"},
        {"C91A-22EF-55B1", "user_2041", "StarLoader", now.addDays(-9), now.addSecs(-660), "ACTIVE"},
        {"77D0-0AC2-91FE", "user_3110", "Neuronot", now.addDays(-2), now.addDays(-1), "REVOKED"},
    };

    sessions = {
        {"user_1029", "StarLoader", "A8F2-91C4", now.addSecs(-60), "ACTIVE"},
        {"user_2041", "StarLoader", "C91A-22EF", now.addSecs(-660), "ACTIVE"},
        {"user_3110", "Neuronot", "77D0-0AC2", now.addDays(-1), "EXPIRED"},
    };

    logs = {
        {now.addSecs(-60), "LICENSE_CREATED", "admin", licenses[0].key.left(14) + "…", "", "SUCCESS"},
        {now.addSecs(-300), "HWID_RESET", "admin", "user_1029", "A8F2-91C4", "SUCCESS"},
        {now.addSecs(-1260), "USER_REVOKED", "admin", "user_2041", "", "SUCCESS"},
        {now.addSecs(-2760), "LICENSE_EXTENDED", "admin", licenses[3].key.left(14) + "…", "", "SUCCESS"},
    };
}

} // namespace KeyStar
