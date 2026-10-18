#include "models/Models.h"

#include <QJsonArray>

namespace KeyStar {

static QDateTime parseDt(const QJsonObject &o, const char *k) {
    const QString s = o.value(QLatin1String(k)).toString();
    QDateTime dt = QDateTime::fromString(s, Qt::ISODateWithMs);
    if (!dt.isValid())
        dt = QDateTime::fromString(s, Qt::ISODate);
    return dt;
}

License License::fromJson(const QJsonObject &o) {
    License l;
    l.key = o.value("key").toString();
    l.product = o.value("product").toString();
    l.status = o.value("status").toString("ACTIVE");
    l.created = parseDt(o, "created");
    l.expires = parseDt(o, "expires");
    l.user = o.value("user").toString();
    l.hwid = o.value("hwid").toString();
    l.devicesUsed = o.value("devicesUsed").toInt();
    l.devicesMax = o.value("devicesMax").toInt(2);
    l.lastAuth = parseDt(o, "lastAuth");
    l.notes = o.value("notes").toString();
    return l;
}

QJsonObject License::toJson() const {
    return {{"key", key}, {"product", product}, {"status", status},
            {"created", created.toString(Qt::ISODate)}, {"expires", expires.toString(Qt::ISODate)},
            {"user", user}, {"hwid", hwid}, {"devicesUsed", devicesUsed},
            {"devicesMax", devicesMax}, {"lastAuth", lastAuth.toString(Qt::ISODate)},
            {"notes", notes}};
}

Product Product::fromJson(const QJsonObject &o) {
    Product p;
    p.name = o.value("name").toString();
    p.identifier = o.value("identifier").toString();
    p.version = o.value("version").toString();
    p.status = o.value("status").toString("ACTIVE");
    p.licenseCount = o.value("licenseCount").toInt();
    p.created = parseDt(o, "created");
    return p;
}

QJsonObject Product::toJson() const {
    return {{"name", name}, {"identifier", identifier}, {"version", version},
            {"status", status}, {"licenseCount", licenseCount},
            {"created", created.toString(Qt::ISODate)}};
}

User User::fromJson(const QJsonObject &o) {
    User u;
    u.username = o.value("username").toString();
    u.licenseCount = o.value("licenseCount").toInt();
    u.deviceCount = o.value("deviceCount").toInt();
    u.status = o.value("status").toString("ACTIVE");
    u.lastLogin = parseDt(o, "lastLogin");
    return u;
}

Device Device::fromJson(const QJsonObject &o) {
    Device d;
    d.hwid = o.value("hwid").toString();
    d.user = o.value("user").toString();
    d.product = o.value("product").toString();
    d.firstSeen = parseDt(o, "firstSeen");
    d.lastSeen = parseDt(o, "lastSeen");
    d.status = o.value("status").toString("ACTIVE");
    return d;
}

Session Session::fromJson(const QJsonObject &o) {
    Session s;
    s.user = o.value("user").toString();
    s.product = o.value("product").toString();
    s.device = o.value("device").toString();
    s.lastActive = parseDt(o, "lastActive");
    s.status = o.value("status").toString("ACTIVE");
    return s;
}

LogEntry LogEntry::fromJson(const QJsonObject &o) {
    LogEntry e;
    e.time = parseDt(o, "time");
    e.action = o.value("action").toString();
    e.actor = o.value("actor").toString();
    e.target = o.value("target").toString();
    e.device = o.value("device").toString();
    e.result = o.value("result").toString("SUCCESS");
    return e;
}

QString fmtDateTime(const QDateTime &dt) {
    if (!dt.isValid())
        return QStringLiteral("—");
    return dt.toString("dd MMM yyyy HH:mm");
}

QString fmtDate(const QDateTime &dt) {
    if (!dt.isValid())
        return QStringLiteral("—");
    return dt.toString("dd MMM yyyy");
}

} // namespace KeyStar
