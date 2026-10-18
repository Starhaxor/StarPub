#include "core/Theme.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPalette>
#include <QStyleFactory>
#include <QWidget>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

namespace KeyStar {

void Theme::apply() {
    // Fusion renders 100% from QSS/palette — no native control leaks through.
    if (QStyleFactory::keys().contains("Fusion"))
        QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Dark base palette so every unstyled surface (viewports, corners) is midnight.
    QPalette pal;
    pal.setColor(QPalette::Window, Colors::Background);
    pal.setColor(QPalette::WindowText, Colors::TextPrimary);
    pal.setColor(QPalette::Base, Colors::Surface);
    pal.setColor(QPalette::AlternateBase, QColor("#0D1526"));
    pal.setColor(QPalette::Text, Colors::TextPrimary);
    pal.setColor(QPalette::Button, Colors::SurfaceElevated);
    pal.setColor(QPalette::ButtonText, Colors::TextPrimary);
    pal.setColor(QPalette::Highlight, QColor("#16294A"));
    pal.setColor(QPalette::HighlightedText, Colors::TextPrimary);
    pal.setColor(QPalette::ToolTipBase, Colors::SurfaceElevated);
    pal.setColor(QPalette::ToolTipText, Colors::TextPrimary);
    pal.setColor(QPalette::PlaceholderText, Colors::TextMuted);
    QApplication::setPalette(pal);

    QApplication::setFont(uiFont(13));
    qApp->setStyleSheet(styleSheet());
}

void Theme::enableDarkFrame(QWidget *w) {
#ifdef Q_OS_WINDOWS
    // DWMWA_USE_IMMERSIVE_DARK_MODE = 20 — runtime linked, no lib dependency.
    HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
    if (!dwm)
        return;
    using Fn = HRESULT(WINAPI *)(HWND, DWORD, LPCVOID, DWORD);
    auto fn = reinterpret_cast<Fn>(GetProcAddress(dwm, "DwmSetWindowAttribute"));
    if (fn) {
        BOOL v = TRUE;
        fn(reinterpret_cast<HWND>(w->winId()), 20, &v, sizeof(v));
    }
#else
    Q_UNUSED(w);
#endif
}

QFont Theme::uiFont(int px, int weight) {
    QFont f;
    f.setFamilies({"Inter", "Segoe UI", "Arial", "sans-serif"});
    f.setPixelSize(px);
    f.setWeight(static_cast<QFont::Weight>(weight <= 0 ? QFont::Normal : weight));
    f.setStyleHint(QFont::SansSerif);
    return f;
}

QFont Theme::monoFont(int px, int weight) {
    QFont f;
    f.setFamilies({"JetBrains Mono", "Cascadia Mono", "Consolas", "Courier New", "monospace"});
    f.setPixelSize(px);
    f.setWeight(static_cast<QFont::Weight>(weight <= 0 ? QFont::Normal : weight));
    f.setStyleHint(QFont::Monospace);
    return f;
}

QColor Theme::statusColor(const QString &status) {
    const QString s = status.toUpper();
    if (s == "ACTIVE" || s == "SUCCESS" || s == "ONLINE" || s == "CONNECTED")
        return Colors::Success;
    if (s == "EXPIRED" || s == "PENDING" || s == "WARNING")
        return Colors::Warning;
    if (s == "REVOKED" || s == "BANNED" || s == "FAILED" || s == "ERROR" || s == "OFFLINE")
        return Colors::Danger;
    return Colors::Info;
}

QString Theme::styleSheet() {
    return QString(R"QSS(
/* ============ KeyStar Control — Midnight Premium (2026) ============ */
* { outline: none; }
QWidget { font-family: "Inter", "Segoe UI", Arial, sans-serif; }

QMainWindow, QDialog, QWidget#AppRoot { background: #070B13; color: #F1F5F9; }
QDialog { border: 1px solid #1E2C43; border-radius: 16px; }
QScrollArea { background: transparent; border: none; }
QScrollArea > QWidget > QWidget { background: transparent; }
QToolTip { background: #131E31; color: #F1F5F9; border: 1px solid #24344F; padding: 7px 10px; border-radius: 8px; }

/* ---------- sidebar : deep navy, floating nav ---------- */
QWidget#Sidebar { background: #0A101B; border-right: 1px solid #16233A; }
QWidget#SideLogoBox { background: qlineargradient(x1:0 y1:0 x2:1 y2:1, stop:0 #22D3EE, stop:1 #8B5CF6); border-radius: 11px; }
QLabel#SideBrand { color: #F1F5F9; font-size: 15px; font-weight: 800; }
QLabel#SideBrandSub { color: #22D3EE; font-size: 10px; font-weight: 700; }
QLabel#SideSection { color: #5B6B82; font-size: 10px; font-weight: 800; padding: 8px 10px 4px 10px; }
QLabel#SideFoot { color: #3D4C63; font-size: 10px; }
QLabel#SideActiveBar { background: qlineargradient(x1:0 y1:0 x2:0 y2:1, stop:0 #22D3EE, stop:1 #8B5CF6); border-radius: 2px; }
QPushButton#SideItem {
    background: transparent; border: 1px solid transparent; border-radius: 10px;
    color: #94A3B8; font-size: 13px; font-weight: 600; text-align: left; padding: 9px 12px;
}
QPushButton#SideItem:hover { background: #111C30; color: #F1F5F9; border: 1px solid #1B2A45; }
QPushButton#SideItem:checked {
    background: qlineargradient(x1:0 y1:0 x2:1 y2:0, stop:0 #16294A, stop:1 #1A2148);
    color: #FFFFFF; font-weight: 700; border: 1px solid #274067;
}

/* ---------- top bar ---------- */
QWidget#TopBar { background: #0A101B; border-bottom: 1px solid #16233A; }
QLabel#TopBrand { color: #F1F5F9; font-size: 13px; font-weight: 800; }
QLabel#TopSearchHint { color: #5B6B82; font-size: 12px; }
QLabel#TopUser { color: #94A3B8; font-size: 12px; font-weight: 600; }
QWidget#Avatar { background: qlineargradient(x1:0 y1:0 x2:1 y2:1, stop:0 #22D3EE, stop:1 #8B5CF6); border-radius: 14px; }
QLabel#AvatarText { color: #060B14; font-size: 12px; font-weight: 800; background: transparent; }
QLabel#ConnPill {
    font-size: 11px; font-weight: 800; padding: 6px 14px;
    border-radius: 14px; background: #0A2621; color: #34D399;
    border: 1px solid #134E3E;
}
QLabel#ConnPill[down="true"] {
    background: #2A1218; color: #FB7185; border: 1px solid #5B2230;
}
QStatusBar { background: #0A101B; color: #5B6B82; border-top: 1px solid #16233A; font-size: 12px; }
QStatusBar::item { border: none; }

/* ---------- type ---------- */
QLabel#PageTitle { color: #F1F5F9; font-size: 24px; font-weight: 800; }
QLabel#PageSub { color: #94A3B8; font-size: 13px; }
QLabel#Caption { color: #5B6B82; font-size: 11px; font-weight: 700; }
QLabel#Secondary { color: #94A3B8; font-size: 13px; }
QLabel#Mono { font-family: "JetBrains Mono", "Cascadia Mono", Consolas, monospace; }

/* ---------- cards : soft elevated, 14px radius ---------- */
QFrame#Card { background: #111A2B; border: 1px solid #1E2C43; border-radius: 14px; }
QFrame#CardHover { background: #111A2B; border: 1px solid #1E2C43; border-radius: 14px; }
QFrame#CardHover:hover { border: 1px solid #2A4A6B; background: #132036; }
QFrame#CardElevated { background: #131E31; border: 1px solid #24344F; border-radius: 16px; }
QFrame#HeroCard { background: qlineargradient(x1:0 y1:0 x2:1 y2:1, stop:0 #0E1B33, stop:0.55 #111A2B, stop:1 #191A3E); border: 1px solid #274067; border-radius: 16px; }
QLabel#MetricTitle { color: #7C8DA6; font-size: 11px; font-weight: 800; }
QLabel#MetricValue { color: #F1F5F9; font-size: 28px; font-weight: 800; }
QLabel#MetricValueCyan { color: #22D3EE; font-size: 28px; font-weight: 800; }
QLabel#MetricDelta { color: #34D399; font-size: 12px; font-weight: 600; }
QLabel#SectionTitle { color: #F1F5F9; font-size: 15px; font-weight: 700; }
QFrame#Divider { background: #1A2942; max-height: 1px; min-height: 1px; border: none; }
QWidget#IconTile { background: #16294A; border: 1px solid #274067; border-radius: 10px; }
QWidget#IconTileCyan { background: #0C2B36; border: 1px solid #155E6B; border-radius: 10px; }
QWidget#AccentLine { background: qlineargradient(x1:0 y1:0 x2:1 y2:0, stop:0 #22D3EE, stop:1 #8B5CF6); border-radius: 2px; }

/* ---------- status pill badge ---------- */
QWidget#StatusPill { border-radius: 12px; padding: 0px; }

/* ---------- buttons : gradient primary ---------- */
QPushButton { border-radius: 10px; font-size: 13px; font-weight: 700; padding: 9px 18px; }
QPushButton#Primary {
    background: qlineargradient(x1:0 y1:0 x2:1 y2:0, stop:0 #22D3EE, stop:1 #8B5CF6);
    color: #060B14; border: none; font-weight: 800; min-height: 22px;
}
QPushButton#Primary:hover { background: qlineargradient(x1:0 y1:0 x2:1 y2:0, stop:0 #67E8F9, stop:1 #A78BFA); }
QPushButton#Primary:pressed { background: qlineargradient(x1:0 y1:0 x2:1 y2:0, stop:0 #0E7490, stop:1 #6D28D9); color: #FFFFFF; }
QPushButton#Primary:disabled { background: #1A2942; color: #5B6B82; }
QPushButton#Ghost {
    background: #131E31; color: #CBD5E1; border: 1px solid #24344F; font-weight: 700; min-height: 22px;
}
QPushButton#Ghost:hover { border: 1px solid #22D3EE; color: #22D3EE; background: #0F2033; }
QPushButton#Ghost:pressed { background: #16294A; color: #F1F5F9; }
QPushButton#Ghost:disabled { color: #5B6B82; border: 1px solid #1A2942; background: transparent; }
QPushButton#Danger {
    background: #1F1420; color: #FB7185; border: 1px solid #5B2230; font-weight: 700; min-height: 22px;
}
QPushButton#Danger:hover { background: #2A1218; border: 1px solid #FB7185; }
QPushButton#Danger:disabled { color: #5B6B82; border: 1px solid #1A2942; }
QPushButton#RowAction {
    background: transparent; color: #94A3B8; border: none; font-size: 12px; font-weight: 700; padding: 6px 10px;
}
QPushButton#RowAction:hover { color: #22D3EE; background: #132036; }
QPushButton#Link { background: transparent; border: none; color: #22D3EE; font-size: 12px; font-weight: 700; padding: 4px 6px; }
QPushButton#Link:hover { color: #67E8F9; }

/* ---------- inputs : 40px tall, soft ---------- */
QLineEdit#Field, QTextEdit#Field, QSpinBox#Field, QComboBox#Field {
    background: #0A1322; color: #F1F5F9;
    border: 1px solid #1E2C43; border-radius: 10px;
    font-size: 13px; padding: 10px 12px; selection-background-color: #155E75;
    min-height: 22px;
}
QLineEdit#Field:focus, QTextEdit#Field:focus, QSpinBox#Field:focus, QComboBox#Field:focus,
QComboBox#Field:on { border: 1px solid #22D3EE; background: #0C1729; }
QLineEdit#Field:disabled, QComboBox#Field:disabled, QSpinBox#Field:disabled {
    color: #5B6B82; background: #0E1624; border: 1px solid #16233A;
}
QLineEdit#FieldInner { background: transparent; border: none; color: #F1F5F9; font-size: 13px; selection-background-color: #155E75; }
QWidget#SearchWrap {
    background: #0A1322; border: 1px solid #1E2C43; border-radius: 12px;
}
QWidget#SearchWrap[focused="true"] { border: 1px solid #22D3EE; background: #0C1729; }
QWidget#TopSearch {
    background: #0E1729; border: 1px solid #1E2C43; border-radius: 10px;
}
QWidget#TopSearch:hover { border: 1px solid #274067; }

/* combobox */
QComboBox#Field { padding-right: 30px; }
QComboBox#Field::drop-down { border: none; width: 26px; }
QComboBox#Field::down-arrow { image: url(:/icons/chevron-down-muted.svg); width: 12px; height: 12px; margin-right: 9px; }
QComboBox#Field QAbstractItemView {
    background: #131E31; color: #F1F5F9; border: 1px solid #24344F; border-radius: 10px;
    selection-background-color: #16294A; selection-color: #FFFFFF;
    outline: none; padding: 4px;
}
QComboBox#Field QAbstractItemView::item { padding: 8px 12px; border: none; border-radius: 6px; }

/* spinbox */
QSpinBox#Field { padding-right: 26px; }
QSpinBox#Field::up-button, QSpinBox#Field::down-button {
    width: 22px; border: none; background: transparent; margin: 2px;
}
QSpinBox#Field::up-button:hover, QSpinBox#Field::down-button:hover { background: #1A2942; border-radius: 4px; }
QSpinBox#Field::up-arrow { image: url(:/icons/chevron-up-muted.svg); width: 10px; height: 10px; }
QSpinBox#Field::down-arrow { image: url(:/icons/chevron-down-muted.svg); width: 10px; height: 10px; }

/* checkbox */
QCheckBox { color: #CBD5E1; font-size: 13px; spacing: 9px; }
QCheckBox::indicator { width: 18px; height: 18px; border-radius: 6px; }
QCheckBox::indicator:unchecked { background: #0A1322; border: 1px solid #2D3F5C; }
QCheckBox::indicator:unchecked:hover { border: 1px solid #22D3EE; }
QCheckBox::indicator:checked {
    background: qlineargradient(x1:0 y1:0 x2:1 y2:1, stop:0 #22D3EE, stop:1 #8B5CF6); border: 1px solid #22D3EE;
    image: url(:/icons/check-dark.svg);
}
QCheckBox:disabled { color: #5B6B82; }

/* ---------- tables : borderless modern ---------- */
QTableWidget#DataTable {
    background: #0B1322; alternate-background-color: #0D1526;
    color: #E2E8F0; gridline-color: transparent;
    border: 1px solid #1E2C43; border-radius: 14px; font-size: 13px;
    selection-background-color: #16294A; selection-color: #FFFFFF;
    outline: none;
}
QTableWidget#DataTable::item { padding: 6px 10px; border: none; border-bottom: 1px solid #13203A; }
QTableWidget#DataTable::item:hover { background: #111D33; }
QTableWidget#DataTable::item:selected { background: #16294A; color: #FFFFFF; }
QTableCornerButton::section { background: #0B1322; border: none; }
QHeaderView { background: transparent; }
QHeaderView::section {
    background: #0B1322; color: #64748B; border: none;
    border-bottom: 1px solid #1E2C43; font-size: 11px; font-weight: 800;
    padding: 12px 10px;
}
QHeaderView::section:hover { color: #F1F5F9; }

/* ---------- splitter / scrollbars ---------- */
QSplitter::handle { background: #1A2942; }
QSplitter::handle:horizontal { width: 1px; }
QSplitter::handle:vertical { height: 1px; }
QSplitter::handle:hover { background: #22D3EE; }
QScrollBar:vertical { background: transparent; width: 8px; margin: 4px 2px 4px 0px; }
QScrollBar::handle:vertical { background: #24344F; border-radius: 4px; min-height: 40px; }
QScrollBar::handle:vertical:hover { background: #22D3EE; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; height: 0; }
QScrollBar:horizontal { background: transparent; height: 8px; margin: 0px 4px 2px 4px; }
QScrollBar::handle:horizontal { background: #24344F; border-radius: 4px; min-width: 40px; }
QScrollBar::handle:horizontal:hover { background: #22D3EE; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; width: 0; }

/* ---------- groups / misc ---------- */
QGroupBox {
    color: #94A3B8; font-size: 11px; font-weight: 800;
    border: 1px solid #1E2C43; border-radius: 14px; background: #111A2B;
    margin-top: 14px; padding: 16px 18px 18px 18px;
}
QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; color: #94A3B8; }
QMenu { background: #131E31; color: #F1F5F9; border: 1px solid #24344F; border-radius: 12px; padding: 6px; }
QMenu::item { padding: 8px 24px 8px 14px; border: none; border-radius: 8px; }
QMenu::item:selected { background: #16294A; }
QTabWidget::pane { border: 1px solid #1E2C43; border-radius: 14px; background: #111A2B; }
QTabBar::tab { background: transparent; color: #94A3B8; padding: 10px 18px; font-size: 13px; font-weight: 700; border: none; border-bottom: 2px solid transparent; }
QTabBar::tab:selected { color: #22D3EE; border-bottom: 2px solid #22D3EE; }
QTabBar::tab:hover { color: #F1F5F9; }
)QSS");
}

} // namespace KeyStar
