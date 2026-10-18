#include "api/KeyStarClient.h"

#include "core/Config.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QTimer>

namespace KeyStar {

KeyStarClient::KeyStarClient(QObject *parent) : QObject(parent) {
    m_base = Config::instance().config().apiEndpoint;
}

QString KeyStarClient::newRequestId() {
    return QStringLiteral("req-%1").arg(
        QRandomGenerator::global()->bounded(100000, 999999));
}

void KeyStarClient::get(const QString &path, Callback cb) {
    send("GET", path, QJsonDocument(), std::move(cb));
}

void KeyStarClient::post(const QString &path, const QJsonDocument &doc, Callback cb) {
    send("POST", path, doc, std::move(cb));
}

void KeyStarClient::ping(Callback cb) {
    send("GET", "/health", QJsonDocument(), std::move(cb));
}

void KeyStarClient::send(const QString &method, const QString &path,
                         const QJsonDocument &doc, Callback cb) {
    ApiResult early;
    if (!isConfigured()) {
        early.ok = false;
        early.errorCode = "KS-NOENDPOINT";
        early.message = "API endpoint is not configured.";
        early.debugDetail = "KeyStarClient::send — empty base URL (mock mode).";
        QTimer::singleShot(0, this, [cb, early]() { cb(early); });
        return;
    }

    const QString requestId = newRequestId();
    QNetworkRequest req(QUrl(m_base + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    req.setRawHeader("X-Request-Id", requestId.toUtf8());
    req.setTransferTimeout(Config::instance().config().requestTimeoutMs);

    emit requestStarted(requestId, method, path);

    auto *timer = new QElapsedTimer();
    timer->start();
    const qint64 t0 = timer->nsecsElapsed();

    QNetworkReply *reply = (method == "POST")
        ? m_nam.post(req, doc.toJson(QJsonDocument::Compact))
        : m_nam.get(req);

    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        ApiResult r;
        r.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        r.elapsedMs = (timer->nsecsElapsed() - t0) / 1000000;
        const QByteArray raw = reply->readAll();
        r.body = QJsonDocument::fromJson(raw);
        if (reply->error() == QNetworkReply::NoError && r.httpStatus < 400) {
            r.ok = true;
            r.message = "Operation completed";
        } else {
            r.ok = false;
            if (reply->error() == QNetworkReply::TimeoutError) {
                r.errorCode = "KS-TIMEOUT";
                r.message = "KeyStar API request timed out.";
            } else if (reply->error() != QNetworkReply::NoError
                       && r.httpStatus == 0) {
                r.errorCode = "KS-NETWORK";
                r.message = "Unable to connect to KeyStar API.";
            } else {
                r.errorCode = QStringLiteral("KS-%1").arg(r.httpStatus);
                r.message = "KeyStar API could not process the request.";
            }
            r.debugDetail = QStringLiteral("[%1] %2 %3 -> HTTP %4 : %5")
                                .arg(requestId, method, path)
                                .arg(r.httpStatus)
                                .arg(QString::fromUtf8(raw.left(512)));
        }
        emit requestFinished(requestId, r.httpStatus, r.elapsedMs, r.ok);
        reply->deleteLater();
        delete timer;
        cb(r);
    });
}

} // namespace KeyStar
