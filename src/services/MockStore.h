#pragma once

#include "models/Models.h"

#include <QObject>
#include <QVector>

namespace KeyStar {

// In-memory seed data so the MVP runs end-to-end without a live API.
// Services read/write here when no endpoint is configured; once a real
// endpoint exists the same service API delegates to KeyStarClient.
class MockStore : public QObject {
    Q_OBJECT
public:
    static MockStore &instance();

    QVector<License> licenses;
    QVector<Product> products;
    QVector<User> users;
    QVector<Device> devices;
    QVector<Session> sessions;
    QVector<LogEntry> logs;

    void seed();
    void addLog(const QString &action, const QString &actor,
                const QString &target = {}, const QString &device = {},
                const QString &result = "SUCCESS");

private:
    MockStore() { seed(); }
};

} // namespace KeyStar
