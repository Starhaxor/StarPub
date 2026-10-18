#include "ui/pages/DashboardPage.h"

#include "core/Config.h"
#include "core/Theme.h"
#include "services/Services.h"
#include "ui/Icons.h"
#include "ui/components/Components.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace KeyStar::Ui {

DashboardPage::DashboardPage(SystemService *sys, QWidget *parent)
    : QWidget(parent), m_sys(sys) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(16);

    // Modern header: title + live clock pill
    auto *head = new QHBoxLayout();
    head->setContentsMargins(0, 0, 0, 0);
    auto *tcol = new QVBoxLayout();
    tcol->setSpacing(4);
    auto *title = new QLabel(QStringLiteral("Dashboard"), this);
    title->setObjectName("PageTitle");
    auto *sub = secondaryLabel("KeyStar license infrastructure at a glance.", this);
    sub->setObjectName("PageSub");
    tcol->addWidget(title);
    tcol->addWidget(sub);
    head->addLayout(tcol);
    head->addStretch(1);
    auto *refBtn = ghostButton("REFRESH", this, "refresh");
    connect(refBtn, &QPushButton::clicked, this, &DashboardPage::refresh);
    head->addWidget(refBtn);
    lay->addLayout(head);

    // Hero banner: gradient card with quick insight
    auto *hero = new QFrame(this);
    hero->setObjectName("HeroCard");
    auto *hl = new QHBoxLayout(hero);
    hl->setContentsMargins(20, 16, 20, 16);
    hl->setSpacing(14);
    auto *htile = new QWidget(hero);
    htile->setObjectName("SideLogoBox");
    htile->setFixedSize(44, 44);
    auto *htl = new QHBoxLayout(htile);
    htl->setContentsMargins(0, 0, 0, 0);
    htl->setAlignment(Qt::AlignCenter);
    auto *hic = new QLabel(htile);
    hic->setPixmap(Icons::pixmap("activity", QColor("#060B14"), 20));
    hic->setStyleSheet("background: transparent; border: none;");
    htl->addWidget(hic);
    auto *htx = new QVBoxLayout();
    htx->setSpacing(2);
    auto *ht1 = new QLabel(QStringLiteral("System nominal — all services operational"), hero);
    ht1->setStyleSheet("color: #F1F5F9; font-size: 14px; font-weight: 800; background: transparent; border: none;");
    auto *ht2 = new QLabel(QStringLiteral("Monitor licenses, sessions and devices in real time. Press Ctrl+K for instant search."), hero);
    ht2->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent; border: none;");
    htx->addWidget(ht1);
    htx->addWidget(ht2);
    hl->addWidget(htile);
    hl->addLayout(htx, 1);
    lay->addWidget(hero);

    auto *grid = new QGridLayout();
    grid->setSpacing(14);
    grid->setContentsMargins(0, 4, 0, 0);
    m_grid = grid;
    m_active = new MetricCard("Active Licenses", this, true);
    m_expired = new MetricCard("Expired Licenses", this);
    m_users = new MetricCard("Total Users", this);
    m_sessions = new MetricCard("Active Sessions", this);
    m_devices = new MetricCard("Registered Devices", this);
    m_cards = {m_active, m_expired, m_users, m_sessions, m_devices};
    layoutMetrics();
    lay->addLayout(grid);

    // Lower row: activity (wide) + system (narrow)
    auto *row = new QHBoxLayout();
    row->setSpacing(14);

    auto *actCard = new QFrame(this);
    actCard->setObjectName("Card");
    auto *al = new QVBoxLayout(actCard);
    al->setContentsMargins(18, 16, 18, 14);
    al->setSpacing(4);
    auto *viewAll = linkButton("VIEW ALL  →", this);
    connect(viewAll, &QPushButton::clicked, this, &DashboardPage::goLogs);
    al->addWidget(new SectionHeader("Recent Activity", this, viewAll));
    m_activityBox = new QWidget(this);
    m_activityBox->setLayout(new QVBoxLayout());
    m_activityBox->layout()->setContentsMargins(0, 6, 0, 0);
    m_activityBox->layout()->setSpacing(0);
    al->addWidget(m_activityBox);
    al->addStretch(1);
    row->addWidget(actCard, 3);

    auto *sysCard = new QFrame(this);
    sysCard->setObjectName("Card");
    auto *sl = new QVBoxLayout(sysCard);
    sl->setContentsMargins(18, 16, 18, 14);
    sl->setSpacing(10);
    auto *openSet = linkButton("SETTINGS  →", this);
    connect(openSet, &QPushButton::clicked, this, &DashboardPage::goSettings);
    sl->addWidget(new SectionHeader("System", this, openSet));
    auto kv = [&](const QString &k, QWidget *v) {
        auto *r = new QHBoxLayout();
        r->setContentsMargins(0, 0, 0, 0);
        auto *kl = captionLabel(k.toUpper(), this);
        kl->setFixedWidth(86);
        r->addWidget(kl);
        r->addWidget(v, 1);
        sl->addLayout(r);
    };
    m_apiBadge = new StatusBadge("LOCAL", this);
    kv("API", m_apiBadge);
    auto *mode = monoLabel("local mode", this);
    mode->setStyleSheet("color: #8B98A5;");
    kv("Mode", mode);
    m_prodLabel = secondaryLabel("—", this);
    kv("Licenses", m_prodLabel);
    auto *ver = monoLabel("v1.0.0", this);
    ver->setStyleSheet("color: #8B98A5;");
    kv("Version", ver);
    sl->addStretch(1);
    row->addWidget(sysCard, 1);

    lay->addLayout(row);
    lay->addStretch(1);
}

void DashboardPage::layoutMetrics() {
    const int cols = (width() < 1080) ? 3 : 5;
    if (cols == m_cols && m_cols != 0)
        return;
    m_cols = cols;
    for (auto *c : m_cards)
        m_grid->removeWidget(c);
    for (int i = 0; i < m_cards.size(); ++i)
        m_grid->addWidget(m_cards[i], i / cols, i % cols);
    for (int i = 0; i < cols; ++i)
        m_grid->setColumnStretch(i, 1);
}

void DashboardPage::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    layoutMetrics();
}

static QColor actionColor(const QString &a) {    if (a.contains("REVOK") || a.contains("DELET") || a.contains("BAN"))
        return Colors::Danger;
    if (a.contains("RESET") || a.contains("EXTEND"))
        return Colors::Warning;
    return Colors::Success;
}

void DashboardPage::refresh() {
    emit loading(true);
    m_sys->dashboard([this](const DashboardStats &s) {
        m_active->setValue(QString::number(s.activeLicenses));
        m_active->setDelta(QStringLiteral("+%1 today").arg(s.createdToday),
                           s.createdToday > 0 ? Colors::Success : Colors::TextMuted);
        m_expired->setValue(QString::number(s.expiredLicenses));
        m_expired->setDelta(s.expiredLicenses ? "needs attention" : "all clear",
                            s.expiredLicenses ? Colors::Warning : Colors::TextMuted);
        m_users->setValue(QString::number(s.totalUsers));
        m_users->setDelta(QString());
        m_sessions->setValue(QString::number(s.activeSessions));
        m_sessions->setDelta(QStringLiteral("live now"),
                             s.activeSessions ? Colors::Success : Colors::TextMuted);
        m_devices->setValue(QString::number(s.registeredDevices));
        m_devices->setDelta(QString());
        m_prodLabel->setText(QStringLiteral("%1 total").arg(s.activeLicenses + s.expiredLicenses));
        emit loading(false);
    });
    m_sys->recentActivity(7, [this](const QVector<LogEntry> &logs) {
        QLayoutItem *it;
        while ((it = m_activityBox->layout()->takeAt(0)) != nullptr) {
            delete it->widget();
            delete it;
        }
        if (logs.isEmpty()) {
            m_activityBox->layout()->addWidget(secondaryLabel("No recent activity.", m_activityBox));
            return;
        }
        bool first = true;
        for (const auto &e : logs) {
            if (!first)
                m_activityBox->layout()->addWidget(divider(m_activityBox));
            first = false;
            auto *row = new QWidget(m_activityBox);
            row->setStyleSheet("background: transparent;");
            auto *hl = new QHBoxLayout(row);
            hl->setContentsMargins(2, 9, 2, 9);
            hl->setSpacing(10);
            auto *dot = new QLabel(QStringLiteral("●"), row);
            const QColor c = actionColor(e.action);
            dot->setStyleSheet(QStringLiteral("color: %1; font-size: 9px; background: transparent; border: none;").arg(c.name()));
            auto *a = new QLabel(e.action.toLower().replace('_', ' '), row);
            a->setFont(Theme::uiFont(13));
            a->setStyleSheet("background: transparent; color: #E2E8F0;");
            auto *tgt = monoLabel(e.target.isEmpty() ? QString() : e.target, row);
            tgt->setStyleSheet("color: #5B6B82; background: transparent; border: none;");
            auto *t = monoLabel(e.time.toString("HH:mm"), row);
            t->setStyleSheet("color: #5B6B82; background: transparent; border: none;");
            hl->addWidget(dot);
            hl->addWidget(a);
            hl->addWidget(tgt, 1);
            hl->addWidget(t);
            m_activityBox->layout()->addWidget(row);
        }
    });
    m_sys->ping([this](bool online, qint64) {
        const bool mock = Config::instance().config().apiEndpoint.isEmpty();
        m_apiBadge->setStatus(!online ? "OFFLINE" : (mock ? "LOCAL" : "ONLINE"));
    });
}

} // namespace KeyStar::Ui
