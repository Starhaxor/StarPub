#pragma once

#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>

namespace KeyStar {

// Spec Section 2 — Color System (Midnight + Cyan/Violet gradient)
struct Colors {
    static inline const QColor Background      = QColor("#070B13");
    static inline const QColor BackgroundAlt   = QColor("#0A101B");
    static inline const QColor Surface         = QColor("#0E1624");
    static inline const QColor SurfaceElevated = QColor("#131E31");
    static inline const QColor Card            = QColor("#111A2B");
    static inline const QColor Border          = QColor("#1E2C43");
    static inline const QColor BorderSoft       = QColor("#16233A");

    static inline const QColor Primary       = QColor("#22D3EE");
    static inline const QColor PrimaryHover  = QColor("#67E8F9");
    static inline const QColor PrimaryMuted  = QColor("#0E7490");
    static inline const QColor Accent        = QColor("#8B5CF6");
    static inline const QColor AccentSoft     = QColor("#A78BFA");

    static inline const QColor TextPrimary   = QColor("#F1F5F9");
    static inline const QColor TextSecondary = QColor("#94A3B8");
    static inline const QColor TextMuted     = QColor("#5B6B82");

    static inline const QColor Success = QColor("#34D399");
    static inline const QColor Warning = QColor("#FBBF24");
    static inline const QColor Danger  = QColor("#FB7185");
    static inline const QColor Info    = QColor("#22D3EE");
};

// Spec Section 3 — Centralized typography (Inter / Segoe UI fallback,
// JetBrains Mono for keys/HWIDs/UUIDs/tokens/timestamps only).
class Theme : public QObject {
    Q_OBJECT
public:
    static void apply();
    static void enableDarkFrame(QWidget *w); // dark native titlebar (Windows)
    static QFont uiFont(int px = 13, int weight = QFont::Normal);
    static QFont monoFont(int px = 13, int weight = QFont::Normal);
    static QColor statusColor(const QString &status);
    static QString styleSheet();
};

} // namespace KeyStar
