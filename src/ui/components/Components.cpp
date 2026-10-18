#include "ui/components/Components.h"

#include "core/Config.h"
#include "core/Theme.h"
#include "ui/Icons.h"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QStyle>
#include <QVBoxLayout>

namespace KeyStar::Ui {

static void polish(QWidget *w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

// ---------- StatusBadge (tinted pill) ----------
StatusBadge::StatusBadge(const QString &status, QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, true);
    m_pill = new QWidget(this);
    m_pill->setAttribute(Qt::WA_StyledBackground, true);
    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_pill);
    outer->addStretch(1);
    auto *lay = new QHBoxLayout(m_pill);
    lay->setContentsMargins(10, 4, 10, 4);
    lay->setSpacing(7);
    m_dot = new QLabel(QStringLiteral("●"), m_pill);
    m_text = new QLabel(m_pill);
    m_text->setFont(Theme::uiFont(11, QFont::DemiBold));
    lay->addWidget(m_dot);
    lay->addWidget(m_text);
    setStatus(status);
}

void StatusBadge::setStatus(const QString &status) {
    m_text->setText(status.toUpper());
    const QColor c = Theme::statusColor(status);
    QColor bg = c;
    bg.setAlpha(30);
    QColor border = c;
    border.setAlpha(120);
    m_pill->setObjectName("StatusPill");
    m_pill->setStyleSheet(QStringLiteral(
        "QWidget#StatusPill { background: %1; border: 1px solid %2; border-radius: 12px; }")
        .arg(bg.name(QColor::HexArgb), border.name(QColor::HexArgb)));
    m_dot->setStyleSheet(QStringLiteral("color: %1; font-size: 9px; font-weight: 800; border: none; background: transparent;").arg(c.name()));
    m_text->setStyleSheet(QStringLiteral("color: %1; border: none; background: transparent; font-weight: 800;").arg(c.name()));
}

// ---------- MetricCard : modern with icon tile + accent ----------
static QString metricIcon(const QString &title) {
    const QString t = title.toLower();
    if (t.contains("active") && t.contains("lic")) return "key";
    if (t.contains("expir")) return "clock";
    if (t.contains("user")) return "users";
    if (t.contains("session")) return "activity";
    if (t.contains("device")) return "monitor";
    if (t.contains("product")) return "package";
    if (t.contains("revenue")) return "trending-up";
    return "grid";
}

MetricCard::MetricCard(const QString &title, QWidget *parent, bool cyanValue)
    : QFrame(parent) {
    setObjectName("Card");
    setAttribute(Qt::WA_StyledBackground, true);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(18, 16, 18, 16);
    lay->setSpacing(8);
    auto *top = new QHBoxLayout();
    top->setContentsMargins(0, 0, 0, 0);
    top->setSpacing(10);
    auto *tile = new QWidget(this);
    tile->setObjectName(cyanValue ? "IconTileCyan" : "IconTile");
    tile->setFixedSize(36, 36);
    auto *tl = new QHBoxLayout(tile);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setAlignment(Qt::AlignCenter);
    auto *ic = new QLabel(tile);
    const QColor icColor = cyanValue ? Colors::Primary : Colors::TextSecondary;
    ic->setPixmap(Icons::pixmap(metricIcon(title), icColor, 17));
    ic->setStyleSheet("background: transparent; border: none;");
    tl->addWidget(ic);
    auto *t = new QLabel(title.toUpper(), this);
    t->setObjectName("MetricTitle");
    t->setWordWrap(true);
    top->addWidget(tile);
    top->addWidget(t, 1);
    lay->addLayout(top);
    m_value = new QLabel(QStringLiteral("—"), this);
    m_value->setObjectName(cyanValue ? "MetricValueCyan" : "MetricValue");
    m_delta = new QLabel(QString(), this);
    m_delta->setObjectName("MetricDelta");
    lay->addWidget(m_value);
    lay->addWidget(m_delta);
    lay->addStretch(1);
}

void MetricCard::setValue(const QString &v) { m_value->setText(v); }

void MetricCard::setDelta(const QString &d, const QColor &c) {
    m_delta->setText(d);
    if (c.isValid())
        m_delta->setStyleSheet(QStringLiteral("color: %1; font-size: 12px;").arg(c.name()));
}

// ---------- SectionHeader ----------
SectionHeader::SectionHeader(const QString &title, QWidget *parent, QWidget *action)
    : QWidget(parent) {
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);
    auto *t = new QLabel(title, this);
    t->setObjectName("SectionTitle");
    lay->addWidget(t);
    lay->addStretch(1);
    if (action)
        lay->addWidget(action);
}

// ---------- IconField ----------
IconField::IconField(const QString &iconName, const QString &placeholder, QWidget *parent)
    : QWidget(parent) {
    setObjectName("SearchWrap");
    setAttribute(Qt::WA_StyledBackground, true);
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(10, 0, 6, 0);
    lay->setSpacing(8);
    auto *ic = new QLabel(this);
    ic->setPixmap(Icons::pixmap(iconName, Colors::TextMuted, 15));
    ic->setStyleSheet("background: transparent; border: none;");
    m_edit = new QLineEdit(this);
    m_edit->setObjectName("FieldInner");
    m_edit->setPlaceholderText(placeholder);
    m_edit->setStyleSheet("QLineEdit#FieldInner { padding: 9px 0px; }");
    lay->addWidget(ic);
    lay->addWidget(m_edit, 1);
    m_edit->installEventFilter(this);
}

void IconField::focusEdit() {
    m_edit->setFocus(Qt::ShortcutFocusReason);
    m_edit->selectAll();
}

bool IconField::eventFilter(QObject *o, QEvent *e) {
    if (o == m_edit && (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut)) {
        setProperty("focused", e->type() == QEvent::FocusIn);
        polish(this);
    }
    return QWidget::eventFilter(o, e);
}

// ---------- SearchBar ----------
SearchBar::SearchBar(const QString &placeholder, QWidget *parent)
    : IconField("search", placeholder, parent) {
    edit()->setClearButtonEnabled(true);
    m_timer.setSingleShot(true);
    m_timer.setInterval(300);
    connect(edit(), &QLineEdit::textChanged, this, [this]() {
        m_timer.stop();
        m_timer.start();
    });
    connect(&m_timer, &QTimer::timeout, this, [this]() { emit debounced(edit()->text()); });
}

void SearchBar::focusSearch() { focusEdit(); }

// ---------- Toast ----------
void Toast::show(QWidget *host, const QString &msg, Kind k) {
    if (!Config::instance().config().notificationsEnabled || !host)
        return;
    auto *t = new Toast(host, msg, k);
    t->QWidget::show();
}

Toast::Toast(QWidget *host, const QString &msg, Kind k) : QWidget(host) {
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_StyledBackground, true);
    const QString glyph = k == Kind::Success ? QStringLiteral("check")
        : k == Kind::Warning                 ? QStringLiteral("clock")
                                             : QStringLiteral("x");
    const QColor c = k == Kind::Success ? Colors::Success
        : k == Kind::Warning            ? Colors::Warning
                                        : Colors::Danger;
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    auto *card = new QFrame(this);
    card->setObjectName("CardElevated");
    auto *cl = new QHBoxLayout(card);
    cl->setContentsMargins(14, 10, 16, 10);
    cl->setSpacing(10);
    auto *bar = new QLabel(card);
    bar->setFixedWidth(3);
    bar->setStyleSheet(QStringLiteral("background: %1; border-radius: 1px;").arg(c.name()));
    auto *ic = new QLabel(card);
    ic->setPixmap(Icons::pixmap(glyph, c, 15));
    ic->setStyleSheet("background: transparent; border: none;");
    auto *lb = new QLabel(msg, card);
    lb->setFont(Theme::uiFont(13));
    lb->setStyleSheet("background: transparent; border: none;");
    cl->addWidget(bar);
    cl->addWidget(ic);
    cl->addWidget(lb);
    lay->addWidget(card);
    adjustSize();
    move((host->width() - width()) / 2, host->height() - height() - 40);
    raise();

    auto *fx = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(fx);
    auto *in = new QPropertyAnimation(fx, "opacity", this);
    in->setDuration(160);
    in->setStartValue(0.0);
    in->setEndValue(1.0);
    in->start(QAbstractAnimation::DeleteWhenStopped);
    QTimer::singleShot(2600, this, [this, fx]() {
        auto *out = new QPropertyAnimation(fx, "opacity", this);
        out->setDuration(250);
        out->setStartValue(1.0);
        out->setEndValue(0.0);
        connect(out, &QPropertyAnimation::finished, this, &QWidget::close);
        out->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

// ---------- LoadingOverlay ----------
LoadingOverlay::LoadingOverlay(QWidget *host, const QString &text)
    : QWidget(host), m_base(text) {
    setAttribute(Qt::WA_StyledBackground, true);
    setGeometry(host->rect());
    setStyleSheet("background: rgba(7, 11, 19, 185); border: none;");
    auto *lay = new QVBoxLayout(this);
    lay->setAlignment(Qt::AlignCenter);
    auto *card = new QFrame(this);
    card->setObjectName("CardElevated");
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(34, 22, 34, 20);
    cl->setSpacing(8);
    m_label = new QLabel(card);
    m_label->setFont(Theme::monoFont(15, QFont::DemiBold));
    m_label->setStyleSheet("color: #22D3EE; background: transparent; border: none;");
    m_label->setAlignment(Qt::AlignCenter);
    auto *sub = captionLabel("CONTACTING KEYSTAR API", card);
    sub->setAlignment(Qt::AlignCenter);
    cl->addWidget(m_label);
    cl->addWidget(sub);
    lay->addWidget(card);
    host->installEventFilter(this);
    auto *tm = new QTimer(this);
    connect(tm, &QTimer::timeout, this, [this]() {
        m_dots = (m_dots + 1) % 4;
        m_label->setText(m_base + QString(m_dots, '.') + QString(3 - m_dots, ' '));
    });
    tm->start(280);
    m_label->setText(m_base);
    show();
    raise();
}

bool LoadingOverlay::eventFilter(QObject *o, QEvent *e) {
    if (o == parent() && e->type() == QEvent::Resize)
        setGeometry(static_cast<QWidget *>(parent())->rect());
    return QWidget::eventFilter(o, e);
}

void LoadingOverlay::finish() {
    close();
    deleteLater();
}

// ---------- Confirms ----------
bool confirmAction(QWidget *parent, const QString &title, const QString &body,
                   const QString &okText, bool danger) {
    QDialog d(parent);
    d.setWindowTitle(title);
    d.setModal(true);
    d.setMinimumWidth(430);
    Theme::enableDarkFrame(&d);
    auto *lay = new QVBoxLayout(&d);
    lay->setContentsMargins(22, 20, 22, 18);
    lay->setSpacing(10);
    auto *t = new QLabel(title.toUpper(), &d);
    t->setObjectName("SectionTitle");
    auto *b = new QLabel(body, &d);
    b->setWordWrap(true);
    b->setObjectName("Secondary");
    auto *row = new QHBoxLayout();
    row->addStretch(1);
    auto *cancel = ghostButton("CANCEL", &d);
    QPushButton *ok = danger ? dangerButton(okText.toUpper(), &d) : primaryButton(okText.toUpper(), &d);
    row->addWidget(cancel);
    row->addWidget(ok);
    lay->addWidget(t);
    lay->addWidget(b);
    lay->addSpacing(6);
    lay->addLayout(row);
    QObject::connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
    return d.exec() == QDialog::Accepted;
}

bool confirmDelete(QWidget *parent, const QString &target) {
    QDialog d(parent);
    d.setWindowTitle("Delete");
    d.setModal(true);
    d.setMinimumWidth(450);
    Theme::enableDarkFrame(&d);
    auto *lay = new QVBoxLayout(&d);
    lay->setContentsMargins(22, 20, 22, 18);
    lay->setSpacing(10);
    auto *t = new QLabel(QStringLiteral("DELETE LICENSE"), &d);
    t->setObjectName("SectionTitle");
    t->setStyleSheet("color: #FF4D5E;");
    auto *b = new QLabel(
        QStringLiteral("This will permanently delete:\n%1\n\nType DELETE to confirm.").arg(target), &d);
    b->setWordWrap(true);
    b->setObjectName("Secondary");
    auto *in = new QLineEdit(&d);
    in->setObjectName("Field");
    in->setPlaceholderText("DELETE");
    in->setFont(Theme::monoFont(13));
    auto *row = new QHBoxLayout();
    row->addStretch(1);
    auto *cancel = ghostButton("CANCEL", &d);
    auto *ok = dangerButton("DELETE", &d);
    ok->setEnabled(false);
    row->addWidget(cancel);
    row->addWidget(ok);
    lay->addWidget(t);
    lay->addWidget(b);
    lay->addWidget(in);
    lay->addSpacing(4);
    lay->addLayout(row);
    QObject::connect(in, &QLineEdit::textChanged, ok, [ok](const QString &s) {
        ok->setEnabled(s.trimmed() == "DELETE");
    });
    QObject::connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
    return d.exec() == QDialog::Accepted;
}

// ---------- Factories ----------
static void applyIcon(QPushButton *b, const QString &icon, const QColor &c) {
    if (icon.isEmpty())
        return;
    b->setIcon(Icons::get(icon, c, 15));
    b->setIconSize(QSize(15, 15));
}

QPushButton *primaryButton(const QString &text, QWidget *parent, const QString &icon) {
    auto *b = new QPushButton(text, parent);
    b->setObjectName("Primary");
    b->setCursor(Qt::PointingHandCursor);
    applyIcon(b, icon, Colors::Background);
    return b;
}

QPushButton *ghostButton(const QString &text, QWidget *parent, const QString &icon) {
    auto *b = new QPushButton(text, parent);
    b->setObjectName("Ghost");
    b->setCursor(Qt::PointingHandCursor);
    applyIcon(b, icon, Colors::TextSecondary);
    return b;
}

QPushButton *dangerButton(const QString &text, QWidget *parent, const QString &icon) {
    auto *b = new QPushButton(text, parent);
    b->setObjectName("Danger");
    b->setCursor(Qt::PointingHandCursor);
    applyIcon(b, icon, Colors::Danger);
    return b;
}

QPushButton *linkButton(const QString &text, QWidget *parent) {
    auto *b = new QPushButton(text, parent);
    b->setObjectName("Link");
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QPushButton *iconButton(const QString &icon, const QString &tip,
                        QWidget *parent, bool danger) {
    auto *b = new QPushButton(parent);
    b->setObjectName(danger ? "Danger" : "Ghost");
    b->setCursor(Qt::PointingHandCursor);
    b->setIcon(Icons::get(icon, danger ? Colors::Danger : Colors::TextSecondary, 16));
    b->setIconSize(QSize(16, 16));
    b->setFixedSize(34, 34);
    b->setToolTip(tip);
    return b;
}

QLineEdit *field(QWidget *parent) {
    auto *e = new QLineEdit(parent);
    e->setObjectName("Field");
    return e;
}

QLabel *monoLabel(const QString &text, QWidget *parent) {
    auto *l = new QLabel(text, parent);
    l->setFont(Theme::monoFont(13));
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return l;
}

QLabel *secondaryLabel(const QString &text, QWidget *parent) {
    auto *l = new QLabel(text, parent);
    l->setObjectName("Secondary");
    l->setWordWrap(true);
    return l;
}

QLabel *captionLabel(const QString &text, QWidget *parent) {
    auto *l = new QLabel(text, parent);
    l->setObjectName("Caption");
    return l;
}

QFrame *divider(QWidget *parent) {
    auto *f = new QFrame(parent);
    f->setObjectName("Divider");
    f->setFrameShape(QFrame::NoFrame);
    f->setFixedHeight(1);
    return f;
}

QWidget *emptyState(const QString &title, const QString &body, QWidget *action,
                    const QString &icon) {
    auto *card = new QFrame();
    card->setObjectName("Card");
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(32, 40, 32, 40);
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(8);
    auto *ic = new QLabel(card);
    ic->setPixmap(Icons::pixmap(icon, Colors::TextMuted, 40));
    ic->setAlignment(Qt::AlignCenter);
    ic->setStyleSheet("background: transparent; border: none;");
    auto *t = new QLabel(title.toUpper(), card);
    t->setObjectName("SectionTitle");
    t->setAlignment(Qt::AlignCenter);
    auto *b = new QLabel(body, card);
    b->setObjectName("Secondary");
    b->setAlignment(Qt::AlignCenter);
    b->setWordWrap(true);
    lay->addWidget(ic);
    lay->addWidget(t);
    lay->addWidget(b);
    if (action) {
        lay->addSpacing(8);
        auto *row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        row->addWidget(action);
        lay->addLayout(row);
    }
    auto *wrap = new QWidget();
    auto *wl = new QVBoxLayout(wrap);
    wl->setAlignment(Qt::AlignCenter);
    wl->addWidget(card);
    return wrap;
}

} // namespace KeyStar::Ui
