#include "ui/Icons.h"

#include <QFile>
#include <QHash>
#include <QPainter>
#include <QSvgRenderer>

namespace KeyStar {

static QHash<QString, QIcon> s_cache;

QPixmap Icons::pixmap(const QString &name, const QColor &color, int size) {
    QFile f(QStringLiteral(":/icons/%1.svg").arg(name));
    QPixmap pm;
    if (f.open(QIODevice::ReadOnly)) {
        QString svg = QString::fromUtf8(f.readAll());
        svg.replace(QStringLiteral("currentColor"), color.name());
        QSvgRenderer r(svg.toUtf8());
        // Render 2x for crisp HiDPI, QIcon downscales.
        pm = QPixmap(size * 2, size * 2);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        r.render(&p);
    } else {
        pm = QPixmap(size, size);
        pm.fill(Qt::transparent);
    }
    return pm;
}

QIcon Icons::get(const QString &name, const QColor &color, int size) {
    const QString k = QStringLiteral("%1|%2|%3").arg(name, color.name()).arg(size);
    auto it = s_cache.find(k);
    if (it != s_cache.end())
        return it.value();
    const QIcon ic(pixmap(name, color, size));
    s_cache.insert(k, ic);
    return ic;
}

QIcon Icons::logo(int size) {
    const QString k = QStringLiteral("logo|%1").arg(size);
    auto it = s_cache.find(k);
    if (it != s_cache.end())
        return it.value();
    QPixmap pm(size * 2, size * 2);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size * 2.0;
    // Modern gradient tile: midnight -> indigo
    QLinearGradient grad(0, 0, s, s);
    grad.setColorAt(0.0, QColor("#0E7490"));
    grad.setColorAt(0.45, QColor("#155E75"));
    grad.setColorAt(1.0, QColor("#4C1D95"));
    p.setBrush(grad);
    p.setPen(QPen(QColor("#22D3EE"), s / 48.0));
    p.drawRoundedRect(QRectF(s / 32.0, s / 32.0, s - s / 16.0, s - s / 16.0),
                      s * 0.28, s * 0.28);
    // Glow dot behind star
    p.setBrush(QColor(34, 211, 238, 60));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(s / 2.0, s / 2.0), s * 0.32, s * 0.32);
    // White 4-point star with cyan tint
    const qreal c = s / 2.0, r1 = s * 0.30, r2 = s * 0.095;
    QPolygonF star;
    for (int i = 0; i < 8; ++i) {
        const qreal a = -3.14159265 / 2.0 + i * 3.14159265 / 4.0;
        const qreal r = (i % 2 == 0) ? r1 : r2;
        star << QPointF(c + r * cos(a), c + r * sin(a));
    }
    QLinearGradient starGrad(c - r1, c - r1, c + r1, c + r1);
    starGrad.setColorAt(0.0, QColor("#FFFFFF"));
    starGrad.setColorAt(1.0, QColor("#67E8F9"));
    p.setBrush(starGrad);
    p.setPen(Qt::NoPen);
    p.drawPolygon(star);
    const QIcon ic(pm);
    s_cache.insert(k, ic);
    return ic;
}

} // namespace KeyStar
