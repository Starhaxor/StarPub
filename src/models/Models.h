#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QVector>

namespace KeyStar {

// Spec Section 16 — models. Plain structs; JSON mapping lives here so
// services/UI never hand-parse QJson.

struct License {
    QString key;
    QString product;
    QString status; // ACTIVE | EXPIRED | REVOKED | PENDING
    QDateTime created;
    QDateTime expires; // invalid = never ("—")
    QString user;
    QString hwid;
    int devicesUsed = 0;
    int devicesMax = 2;
    QDateTime lastAuth;
    QString notes;

    static License fromJson(const QJsonObject &o);
    QJsonObject toJson() const;
    bool isExpired() const { return expires.isValid() && expires < QDateTime::currentDateTime(); }
};

struct Product {
    QString name;
    QString identifier;
    QString version;
    QString status; // ACTIVE | DISABLED
    int licenseCount = 0;
    QDateTime created;

    static Product fromJson(const QJsonObject &o);
    QJsonObject toJson() const;
};

struct User {
    QString username;
    int licenseCount = 0;
    int deviceCount = 0;
    QString status; // ACTIVE | REVOKED | BANNED
    QDateTime lastLogin;

    static User fromJson(const QJsonObject &o);
};

struct Device {
    QString hwid;
    QString user;
    QString product;
    QDateTime firstSeen;
    QDateTime lastSeen;
    QString status; // ACTIVE | REVOKED

    static Device fromJson(const QJsonObject &o);
};

struct Session {
    QString user;
    QString product;
    QString device; // short HWID
    QDateTime lastActive;
    QString status; // ACTIVE | EXPIRED

    static Session fromJson(const QJsonObject &o);
};

struct LogEntry {
    QDateTime time;
    QString action; // LICENSE_CREATED, HWID_RESET, ...
    QString actor;
    QString target;
    QString device;
    QString result; // SUCCESS | FAILED

    static LogEntry fromJson(const QJsonObject &o);
};

struct DashboardStats {
    int activeLicenses = 0;
    int expiredLicenses = 0;
    int totalUsers = 0;
    int activeSessions = 0;
    int registeredDevices = 0;
    int createdToday = 0;
};

QString fmtDateTime(const QDateTime &dt);
QString fmtDate(const QDateTime &dt);

} // namespace KeyStar
