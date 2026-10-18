#include "ui/pages/SystemPages.h"

#include "core/Config.h"
#include "core/Theme.h"
#include "models/Models.h"
#include "services/Services.h"
#include "ui/components/Components.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>

namespace KeyStar::Ui {

static QTableWidget *makeTable(const QStringList &cols, QWidget *parent) {
    auto *t = new QTableWidget(parent);
    t->setObjectName("DataTable");
    t->setColumnCount(cols.size());
    t->setHorizontalHeaderLabels(cols);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setShowGrid(false);
    t->setWordWrap(false);
    t->setAlternatingRowColors(true);
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(36);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->setSortingEnabled(true);
    return t;
}

static void pageHead(QVBoxLayout *lay, QWidget *self, const QString &title,
                     const QString &sub) {
    auto *t = new QLabel(title, self);
    t->setObjectName("PageTitle");
    lay->addWidget(t);
    lay->addWidget(secondaryLabel(sub, self));
}

static QTableWidgetItem *cellItem(const QString &s) {
    return new QTableWidgetItem(s);
}

static QTableWidgetItem *statusItem(const QString &s) {
    auto *it = new QTableWidgetItem(QStringLiteral("● ") + s);
    it->setForeground(Theme::statusColor(s));
    it->setFont(Theme::uiFont(12, QFont::DemiBold));
    return it;
}

static QTableWidgetItem *monoItem(const QString &s) {
    auto *it = new QTableWidgetItem(s);
    it->setFont(Theme::monoFont(12));
    return it;
}

// ---- Users ----
UsersPage::UsersPage(SystemService *sys, QWidget *parent) : QWidget(parent), m_sys(sys) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(12);
    pageHead(lay, this, "Users", "Registered KeyStar users and their footprint.");
    m_table = makeTable({"USER", "LICENSES", "DEVICES", "STATUS", "LAST LOGIN"}, this);
    lay->addWidget(m_table, 1);
    refresh();
}

void UsersPage::refresh() {
    emit loading(true);
    m_sys->users([this](const QVector<User> &us) {
        emit loading(false);
        m_table->setSortingEnabled(false);
        m_table->setRowCount(us.size());
        for (int r = 0; r < us.size(); ++r) {
            const auto &u = us[r];
            m_table->setItem(r, 0, cellItem(u.username));
            m_table->setItem(r, 1, cellItem(QString::number(u.licenseCount)));
            m_table->setItem(r, 2, cellItem(QString::number(u.deviceCount)));
            m_table->setItem(r, 3, statusItem(u.status));
            m_table->setItem(r, 4, cellItem(fmtDateTime(u.lastLogin)));
        }
        m_table->setSortingEnabled(true);
    });
}

// ---- Devices ----
DevicesPage::DevicesPage(SystemService *sys, QWidget *parent) : QWidget(parent), m_sys(sys) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(12);
    pageHead(lay, this, "Devices", "Hardware bindings seen by the licensing API.");
    m_table = makeTable({"HWID", "USER", "PRODUCT", "FIRST SEEN", "LAST SEEN", "STATUS"}, this);
    lay->addWidget(m_table, 1);
    refresh();
}

void DevicesPage::refresh() {
    emit loading(true);
    m_sys->devices([this](const QVector<Device> &ds) {
        emit loading(false);
        m_table->setSortingEnabled(false);
        m_table->setRowCount(ds.size());
        for (int r = 0; r < ds.size(); ++r) {
            const auto &d = ds[r];
            auto *hw = monoItem(d.hwid);
            hw->setForeground(Colors::TextPrimary);
            m_table->setItem(r, 0, hw);
            m_table->setItem(r, 1, cellItem(d.user));
            m_table->setItem(r, 2, cellItem(d.product));
            m_table->setItem(r, 3, cellItem(fmtDate(d.firstSeen)));
            m_table->setItem(r, 4, cellItem(fmtDateTime(d.lastSeen)));
            m_table->setItem(r, 5, statusItem(d.status));
        }
        m_table->setSortingEnabled(true);
    });
}

// ---- Sessions ----
SessionsPage::SessionsPage(SystemService *sys, QWidget *parent)
    : QWidget(parent), m_sys(sys) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(12);
    pageHead(lay, this, "Sessions", "Live authentication sessions across products.");
    m_table = makeTable({"USER", "PRODUCT", "DEVICE", "LAST ACTIVE", "STATUS"}, this);
    lay->addWidget(m_table, 1);
    refresh();
}

void SessionsPage::refresh() {
    emit loading(true);
    m_sys->sessions([this](const QVector<Session> &ss) {
        emit loading(false);
        m_table->setSortingEnabled(false);
        m_table->setRowCount(ss.size());
        for (int r = 0; r < ss.size(); ++r) {
            const auto &s = ss[r];
            m_table->setItem(r, 0, cellItem(s.user));
            m_table->setItem(r, 1, cellItem(s.product));
            m_table->setItem(r, 2, monoItem(s.device));
            m_table->setItem(r, 3, cellItem(fmtDateTime(s.lastActive)));
            auto *st = new QTableWidgetItem(
                s.status == "ACTIVE" ? QStringLiteral("● ACTIVE") : QStringLiteral("○ EXPIRED"));
            st->setForeground(Theme::statusColor(s.status));
            st->setFont(Theme::uiFont(12, QFont::DemiBold));
            m_table->setItem(r, 4, st);
        }
        m_table->setSortingEnabled(true);
    });
}

// ---- Logs ----
LogsPage::LogsPage(SystemService *sys, QWidget *parent) : QWidget(parent), m_sys(sys) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(12);
    pageHead(lay, this, "Logs", "Read-only audit trail of admin operations.");

    auto *bar = new QHBoxLayout();
    bar->setSpacing(10);
    m_search = new SearchBar("Filter action, actor, target…", this);
    m_search->setMinimumWidth(280);
    m_resultBox = new QComboBox(this);
    m_resultBox->setObjectName("Field");
    m_resultBox->setFixedWidth(140);
    m_resultBox->addItems({"ALL", "SUCCESS", "FAILED"});
    bar->addWidget(m_search, 1);
    bar->addWidget(m_resultBox);
    lay->addLayout(bar);

    auto *split = new QSplitter(Qt::Horizontal, this);
    m_table = makeTable({"TIME", "ACTION", "ACTOR", "TARGET", "RESULT"}, this);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    split->addWidget(m_table);

    auto *detailCard = new QFrame(this);
    detailCard->setObjectName("Card");
    detailCard->setMinimumWidth(260);
    detailCard->setMaximumWidth(320);
    auto *dl = new QVBoxLayout(detailCard);
    dl->setContentsMargins(18, 16, 18, 14);
    dl->setSpacing(8);
    m_detail = new QWidget(detailCard);
    m_detail->setLayout(new QVBoxLayout());
    m_detail->layout()->setContentsMargins(0, 0, 0, 0);
    m_detail->layout()->setSpacing(6);
    m_detail->layout()->addWidget(secondaryLabel("Select a log entry to inspect it.", m_detail));
    dl->addWidget(m_detail);
    split->addWidget(detailCard);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 1);
    lay->addWidget(split, 1);

    connect(m_search, &SearchBar::debounced, this, [this](const QString &) { fillTable(); });
    connect(m_resultBox, &QComboBox::currentTextChanged, this, [this](const QString &) { fillTable(); });
    connect(m_table, &QTableWidget::cellClicked, this, [this](int r, int) {
        if (r >= 0 && r < m_shown.size())
            showDetail(m_shown[r]);
    });
    refresh();
}

void LogsPage::showDetail(const KeyStar::LogEntry &e) {
    QLayoutItem *it;
    while ((it = m_detail->layout()->takeAt(0)) != nullptr) {
        delete it->widget();
        delete it;
    }
    auto *lay = static_cast<QVBoxLayout *>(m_detail->layout());
    lay->addWidget(new SectionHeader("Log Detail", m_detail));
    auto add = [&](const QString &k, const QString &v, bool mono = false) {
        lay->addWidget(captionLabel(k.toUpper(), m_detail));
        QWidget *w = mono ? static_cast<QWidget *>(monoLabel(v, m_detail))
                          : static_cast<QWidget *>(new QLabel(v, m_detail));
        w->setStyleSheet("color: #E6EDF3; font-size: 13px; background: transparent;");
        lay->addWidget(w);
    };
    add("Action", e.action, true);
    add("Time", fmtDateTime(e.time));
    add("Actor", e.actor);
    add("Target", e.target.isEmpty() ? "—" : e.target, true);
    add("Device", e.device.isEmpty() ? "—" : e.device, true);
    lay->addWidget(captionLabel("RESULT", m_detail));
    lay->addWidget(new StatusBadge(e.result, m_detail));
    lay->addStretch(1);
}

void LogsPage::fillTable() {
    const QString q = m_search->text().toLower();
    const QString res = m_resultBox->currentText();
    m_shown.clear();
    for (const auto &e : m_all) {
        if (res != "ALL" && e.result != res)
            continue;
        if (!q.isEmpty() && !e.action.toLower().contains(q)
            && !e.actor.toLower().contains(q) && !e.target.toLower().contains(q))
            continue;
        m_shown.push_back(e);
    }
    m_table->setSortingEnabled(false);
    m_table->setRowCount(m_shown.size());
    for (int r = 0; r < m_shown.size(); ++r) {
        const auto &e = m_shown[r];
        auto *t = monoItem(e.time.toString("HH:mm"));
        t->setForeground(Colors::TextMuted);
        m_table->setItem(r, 0, t);
        m_table->setItem(r, 1, monoItem(e.action));
        m_table->setItem(r, 2, cellItem(e.actor));
        m_table->setItem(r, 3, monoItem(e.target));
        auto *resIt = new QTableWidgetItem(e.result);
        resIt->setForeground(Theme::statusColor(e.result == "SUCCESS" ? "ACTIVE" : "FAILED"));
        resIt->setFont(Theme::uiFont(12, QFont::DemiBold));
        m_table->setItem(r, 4, resIt);
    }
    m_table->setSortingEnabled(true);
}

void LogsPage::refresh() {
    emit loading(true);
    m_sys->logs([this](const QVector<LogEntry> &ls) {
        emit loading(false);
        m_all = ls;
        fillTable();
    });
}

// ---- Settings ----
SettingsPage::SettingsPage(SystemService *sys, QWidget *parent)
    : QWidget(parent), m_sys(sys) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(12);
    pageHead(lay, this, "Settings", "Local console preferences. Secrets are never stored here.");

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *center = new QWidget();
    auto *cl = new QHBoxLayout(center);
    cl->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget();
    body->setMaximumWidth(700);
    auto *bl = new QVBoxLayout(body);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(14);

    auto *g = new QGroupBox("GENERAL", body);
    auto *gf = new QFormLayout(g);
    gf->setSpacing(10);
    m_startup = new QComboBox(g);
    m_startup->setObjectName("Field");
    m_startup->addItems({"Dashboard", "Licenses", "Products", "Logs"});
    m_startup->setMinimumWidth(220);
    m_notif = new QCheckBox("Enable toast notifications", g);
    m_polling = new QCheckBox("Auto-refresh data (polling)", g);
    m_pollSec = new QSpinBox(g);
    m_pollSec->setObjectName("Field");
    m_pollSec->setRange(15, 300);
    m_pollSec->setSuffix(" sec");
    m_pollSec->setMinimumWidth(140);
    gf->addRow("Startup page", m_startup);
    gf->addRow("", m_notif);
    gf->addRow("", m_polling);
    gf->addRow("Polling interval", m_pollSec);
    bl->addWidget(g);

    auto *a = new QGroupBox("API CONNECTION", body);
    auto *af = new QFormLayout(a);
    af->setSpacing(10);
    m_endpoint = new QLineEdit(a);
    m_endpoint->setObjectName("Field");
    m_endpoint->setPlaceholderText("https://api.keystar.local  —  empty = local mode");
    m_timeout = new QSpinBox(a);
    m_timeout->setObjectName("Field");
    m_timeout->setRange(2000, 60000);
    m_timeout->setSuffix(" ms");
    m_timeout->setMinimumWidth(140);
    m_connLabel = new QLabel(a);
    m_connLabel->setObjectName("Secondary");
    auto *testBtn = ghostButton("TEST CONNECTION", a, "activity");
    af->addRow("API Endpoint", m_endpoint);
    af->addRow("Request timeout", m_timeout);
    af->addRow("Status", m_connLabel);
    af->addRow("", testBtn);
    bl->addWidget(a);
    connect(testBtn, &QPushButton::clicked, this, [this]() {
        m_connLabel->setText("Probing…");
        m_sys->ping([this](bool online, qint64 ms) {
            m_connLabel->setText(online ? QStringLiteral("● ONLINE  (%1 ms)").arg(ms)
                                        : QStringLiteral("✕ OFFLINE"));
            emit connectionChanged();
        });
    });

    auto *s = new QGroupBox("SECURITY", body);
    auto *sf = new QFormLayout(s);
    sf->setSpacing(10);
    m_sessionMin = new QSpinBox(s);
    m_sessionMin->setObjectName("Field");
    m_sessionMin->setRange(5, 480);
    m_sessionMin->setSuffix(" min");
    m_sessionMin->setMinimumWidth(140);
    auto *cred = secondaryLabel("Credentials live in OS storage (Phase 2). Nothing secret is written to disk or logs.", s);
    cred->setWordWrap(true);
    sf->addRow("Session timeout", m_sessionMin);
    sf->addRow("Credential storage", cred);
    bl->addWidget(s);

    auto *dv = new QGroupBox("DEVELOPER", body);
    auto *df = new QFormLayout(dv);
    df->setSpacing(6);
    m_devMode = new QCheckBox("Developer Mode — endpoint, request ID, status, latency", dv);
    m_debugLog = new QCheckBox("Debug logging", dv);
    df->addRow("", m_devMode);
    df->addRow("", m_debugLog);
    bl->addWidget(dv);
    bl->addStretch(1);

    cl->addWidget(body);
    cl->addStretch(1);
    scroll->setWidget(center);
    lay->addWidget(scroll, 1);

    auto *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->addStretch(1);
    auto *saveBtn = primaryButton("SAVE SETTINGS", this, "check");
    row->addWidget(saveBtn);
    lay->addLayout(row);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsPage::save);

    const AppConfig &c = Config::instance().config();
    m_endpoint->setText(c.apiEndpoint);
    m_timeout->setValue(c.requestTimeoutMs);
    m_startup->setCurrentText(c.startupPage);
    m_notif->setChecked(c.notificationsEnabled);
    m_polling->setChecked(c.pollingEnabled);
    m_pollSec->setValue(c.pollingIntervalSec);
    m_sessionMin->setValue(c.sessionTimeoutMin);
    m_devMode->setChecked(c.developerMode);
    m_debugLog->setChecked(c.debugLogging);
    m_connLabel->setText(c.apiEndpoint.isEmpty() ? "● LOCAL MODE — no endpoint configured"
                                                 : "Not probed yet — run TEST CONNECTION");
}

void SettingsPage::save() {
    auto &c = Config::instance().config();
    c.apiEndpoint = m_endpoint->text().trimmed();
    c.requestTimeoutMs = m_timeout->value();
    c.startupPage = m_startup->currentText();
    c.notificationsEnabled = m_notif->isChecked();
    c.pollingEnabled = m_polling->isChecked();
    c.pollingIntervalSec = m_pollSec->value();
    c.sessionTimeoutMin = m_sessionMin->value();
    c.developerMode = m_devMode->isChecked();
    c.debugLogging = m_debugLog->isChecked();
    Config::instance().save();
    emit toast("✓ Settings saved", 0);
    emit connectionChanged();
}

} // namespace KeyStar::Ui
