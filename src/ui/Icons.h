#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

namespace KeyStar {

// Recolorable SVG icon set (:/icons/*.svg use stroke="currentColor").
// Sidebar/page icons are real glyphs now — no unicode placeholders.
class Icons {
public:
    static QIcon get(const QString &name, const QColor &color, int size = 18);
    static QPixmap pixmap(const QString &name, const QColor &color, int size);
    static QIcon logo(int size = 32);
};

} // namespace KeyStar
