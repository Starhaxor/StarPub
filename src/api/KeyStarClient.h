#pragma once

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

namespace KeyStar {

// Spec Section 16/18 — KeyStarClient.
// Qt UI must NEVER do HTTP directly; all traffic goes through here.
// Async only (QNetworkAccessManager), never blocks the UI thread.
// On transport/API errors the UI shows a friendly message + KS error code
// (spec 30); raw technical detail is exposed only in Developer Mode.
struct ApiResult {
    bool ok = false;
    QString errorCode;   // e.g. "KS-401", "KS-NETWORK", "KS-TIMEOUT"
    QString message;     // user-safe message
    QString debugDetail; // shown only in Developer Mode / debug logs
    QJsonDocument body;
    int httpStatus = 0;
    qint64 elapsedMs = 0;
};

class KeyStarClient : public QObject {
    Q_OBJECT
public:
    explicit KeyStarClient(QObject *parent = nullptr);

    void setBaseUrl(const QString &url) { m_base = url; }
    void setToken(const QString &token) { m_token = token; }
    QString baseUrl() const { return m_base; }
    bool isConfigured() const { return !m_base.isEmpty(); }

    using Callback = std::function<void(const ApiResult &)>;

    void get(const QString &path, Callback cb);
    void post(const QString &path, const QJsonDocument &doc, Callback cb);
    void ping(Callback cb); // GET /health — drives CONNECTED indicator

signals:
    void requestStarted(QString requestId, QString method, QString path);
    void requestFinished(QString requestId, int httpStatus, qint64 elapsedMs, bool ok);

private:
    void send(const QString &method, const QString &path, const QJsonDocument &doc, Callback cb);
    static QString newRequestId();

    QNetworkAccessManager m_nam;
    QString m_base;
    QString m_token;
};

} // namespace KeyStar
