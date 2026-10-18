#include "ui/MainWindow.h"

#include "api/KeyStarClient.h"
#include "core/Config.h"
#include "core/CredentialStore.h"
#include "core/Theme.h"
#include "services/Services.h"
#include "ui/Icons.h"
#include "ui/components/Components.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/LicensesPage.h"
#include "ui/pages/ProductsPage.h"
#include "ui/pages/SystemPages.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QShowEvent>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

namespace KeyStar::Ui {

static QString pageIcon(const QString &name) {
    if (name == "Dashboard") return "grid";
    if (name == "Licenses") return "key";
    if (name == "Products") return "package";
    if (name == "Users") return "users";
    if (name == "Devices") return "monitor";
    if (name == "Sessions") return "activity";
    if (name == "Logs") return "list";
    return "sliders";
}

MainWindow::MainWindow(KeyStarClient *client, LicenseService *lics,
                       ProductService *prods, SystemService *sys, QWidget *parent)
    : QMainWindow(parent), m_client(client), m_lics(lics), m_sys(sys) {
    setWindowTitle("KeyStar Control");
    setWindowIcon(Icons::logo(32));
    setMinimumSize(1200, 720);
    resize(1440, 900);
    Theme::enableDarkFrame(this);

    m_root = new QWidget(this);
    m_root->setObjectName("AppRoot");
    setCentralWidget(m_root);

    auto *outer = new QVBoxLayout(m_root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    buildTopBar();

    auto *midWrap = new QWidget(m_root);
    auto *mid = new QHBoxLayout(midWrap);
    mid->setContentsMargins(0, 0, 0, 0);
    mid->setSpacing(0);
    buildSidebar();
    m_stack = new QStackedWidget(m_root);
    m_dash = new DashboardPage(sys, m_stack);
    m_licenses = new LicensesPage(lics, prods, m_stack);
    m_products = new ProductsPage(prods, m_stack);
    m_users = new UsersPage(sys, m_stack);
    m_devices = new DevicesPage(sys, m_stack);
    m_sessions = new SessionsPage(sys, m_stack);
    m_logs = new LogsPage(sys, m_stack);
    m_settings = new SettingsPage(sys, m_stack);
    m_order = {"Dashboard", "Licenses", "Products", "Users",
               "Devices", "Sessions", "Logs", "Settings"};
    m_stack->addWidget(m_dash);
    m_stack->addWidget(m_licenses);
    m_stack->addWidget(m_products);
    m_stack->addWidget(m_users);
    m_stack->addWidget(m_devices);
    m_stack->addWidget(m_sessions);
    m_stack->addWidget(m_logs);
    m_stack->addWidget(m_settings);

    mid->addWidget(findChild<QWidget *>("Sidebar"));
    mid->addWidget(m_stack, 1);
    outer->addWidget(midWrap, 1);

    m_apiStatus = new QLabel("API: …", this);
    m_versionLabel = new QLabel("KeyStar  v1.0.0", this);
    m_versionLabel->setStyleSheet("color: #5E6A75;");
    statusBar()->addWidget(m_apiStatus, 1);
    statusBar()->addPermanentWidget(m_versionLabel);

    auto hook = [this](QWidget *p) {
        connect(p, SIGNAL(toast(QString, int)), this, SLOT(showToast(QString, int)));
        connect(p, SIGNAL(loading(bool)), this, SLOT(setLoading(bool)));
    };
    hook(m_dash);
    hook(m_licenses);
    hook(m_products);
    hook(m_logs);
    hook(m_settings);
    connect(m_users, &UsersPage::loading, this, &MainWindow::setLoading);
    connect(m_devices, &DevicesPage::loading, this, &MainWindow::setLoading);
    connect(m_sessions, &SessionsPage::loading, this, &MainWindow::setLoading);
    connect(m_settings, &SettingsPage::connectionChanged, this, &MainWindow::updateConnection);
    // Cross-page navigation
    connect(m_dash, &DashboardPage::goLogs, this, [this]() { goTo("Logs"); });
    connect(m_dash, &DashboardPage::goSettings, this, [this]() { goTo("Settings"); });
    connect(m_products, &ProductsPage::viewLicenses, this,
            [this](const QString &product) {
                goTo("Licenses");
                m_licenses->setProductFilter(product);
            });

    auto *scSearch = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(scSearch, &QShortcut::activated, this, [this]() {
        goTo("Licenses");
        m_licenses->focusSearch();
    });
    auto *scRefresh = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(scRefresh, &QShortcut::activated, this, &MainWindow::refreshCurrent);
    auto *scNew = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(scNew, &QShortcut::activated, this, [this]() { goTo("Licenses"); });
    auto *scLic = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(scLic, &QShortcut::activated, this, [this]() { goTo("Licenses"); });

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &MainWindow::refreshCurrent);

    goTo(Config::instance().config().startupPage);
    updateConnection();
}

void MainWindow::buildTopBar() {
    auto *bar = new QWidget(m_root);
    bar->setObjectName("TopBar");
    bar->setAttribute(Qt::WA_StyledBackground, true);
    bar->setFixedHeight(64);
    auto *lay = new QHBoxLayout(bar);
    lay->setContentsMargins(20, 0, 20, 0);
    lay->setSpacing(12);

    // Command-K search pill (modern SaaS feel)
    auto *search = new QWidget(bar);
    search->setObjectName("TopSearch");
    search->setAttribute(Qt::WA_StyledBackground, true);
    search->setFixedHeight(38);
    search->setMinimumWidth(320);
    search->setMaximumWidth(440);
    search->setCursor(Qt::PointingHandCursor);
    auto *sl = new QHBoxLayout(search);
    sl->setContentsMargins(12, 0, 10, 0);
    sl->setSpacing(8);
    auto *sIc = new QLabel(search);
    sIc->setPixmap(Icons::pixmap("search", Colors::TextMuted, 14));
    sIc->setStyleSheet("background: transparent; border: none;");
    auto *sTx = new QLabel(QStringLiteral("Search licenses, users, HWID…"), search);
    sTx->setObjectName("TopSearchHint");
    sTx->setStyleSheet("background: transparent; border: none;");
    auto *sKbd = new QLabel(QStringLiteral("Ctrl K"), search);
    sKbd->setStyleSheet("background: #16294A; color: #7C8DA6; font-size: 10px; font-weight: 800; "
                        "border: 1px solid #274067; border-radius: 6px; padding: 3px 7px;");
    sl->addWidget(sIc);
    sl->addWidget(sTx, 1);
    sl->addWidget(sKbd);
    search->installEventFilter(this);
    sIc->installEventFilter(this);
    sTx->installEventFilter(this);
    sKbd->installEventFilter(this);
    lay->addWidget(search);
    search->setProperty("searchPill", true);

    lay->addStretch(1);
    const QString uname = CredentialStore::instance().lastUsername();
    // Avatar with gradient + initial
    auto *avWrap = new QWidget(bar);
    avWrap->setObjectName("Avatar");
    avWrap->setFixedSize(32, 32);
    auto *avl = new QHBoxLayout(avWrap);
    avl->setContentsMargins(0, 0, 0, 0);
    avl->setAlignment(Qt::AlignCenter);
    auto *avT = new QLabel(uname.isEmpty() ? QStringLiteral("A") : uname.left(1).toUpper(), avWrap);
    avT->setObjectName("AvatarText");
    avT->setAlignment(Qt::AlignCenter);
    avl->addWidget(avT);
    auto *col = new QVBoxLayout();
    col->setSpacing(0);
    auto *user = new QLabel(uname.isEmpty() ? QStringLiteral("Admin") : uname, bar);
    user->setObjectName("TopUser");
    auto *role = new QLabel(QStringLiteral("ADMINISTRATOR"), bar);
    role->setStyleSheet("color: #5B6B82; font-size: 10px; font-weight: 800; background: transparent;");
    col->addWidget(user);
    col->addWidget(role);
    m_connText = new QLabel(QStringLiteral("CONNECTING"), bar);
    m_connText->setObjectName("ConnPill");
    m_connText->setFixedHeight(30);
    m_connText->setAlignment(Qt::AlignCenter);
    lay->addWidget(avWrap);
    lay->addLayout(col);
    lay->addSpacing(6);
    lay->addWidget(m_connText);
    static_cast<QVBoxLayout *>(m_root->layout())->addWidget(bar);

    // pill click -> Ctrl+K behavior
    // installed via eventFilter below
    m_searchPill = search;
}

void MainWindow::buildSidebar() {
    auto *side = new QWidget(m_root);
    side->setObjectName("Sidebar");
    side->setAttribute(Qt::WA_StyledBackground, true);
    side->setFixedWidth(248);
    auto *lay = new QVBoxLayout(side);
    lay->setContentsMargins(16, 18, 16, 14);
    lay->setSpacing(2);

    auto *brandRow = new QHBoxLayout();
    brandRow->setSpacing(12);
    // gradient logo tile
    auto *logoBox = new QWidget(side);
    logoBox->setObjectName("SideLogoBox");
    logoBox->setFixedSize(38, 38);
    auto *ll = new QHBoxLayout(logoBox);
    ll->setContentsMargins(0, 0, 0, 0);
    ll->setAlignment(Qt::AlignCenter);
    auto *logo = new QLabel(logoBox);
    logo->setPixmap(Icons::logo(24).pixmap(24, 24));
    logo->setFixedSize(24, 24);
    logo->setStyleSheet("background: transparent; border: none;");
    ll->addWidget(logo);
    auto *bt = new QVBoxLayout();
    bt->setSpacing(0);
    auto *brand = new QLabel(QStringLiteral("KEYSTAR"), side);
    brand->setObjectName("SideBrand");
    auto *sub = new QLabel(QStringLiteral("CONTROL CENTER"), side);
    sub->setObjectName("SideBrandSub");
    bt->addWidget(brand);
    bt->addWidget(sub);
    brandRow->addWidget(logoBox);
    brandRow->addLayout(bt);
    brandRow->addStretch(1);
    lay->addLayout(brandRow);
    lay->addSpacing(18);

    auto section = [&](const QString &s) {
        auto *l = new QLabel(s, side);
        l->setObjectName("SideSection");
        lay->addWidget(l);
    };
    lay->addWidget(sideItem("Dashboard", "grid"));
    lay->addSpacing(6);
    section("LICENSE");
    lay->addWidget(sideItem("Licenses", "key"));
    lay->addWidget(sideItem("Products", "package"));
    lay->addSpacing(6);
    section("USERS");
    lay->addWidget(sideItem("Users", "users"));
    lay->addWidget(sideItem("Devices", "monitor"));
    lay->addWidget(sideItem("Sessions", "activity"));
    lay->addSpacing(6);
    section("SYSTEM");
    lay->addWidget(sideItem("Logs", "list"));
    lay->addWidget(sideItem("Settings", "sliders"));
    lay->addStretch(1);
    // Pro tip card
    auto *tip = new QFrame(side);
    tip->setObjectName("HeroCard");
    auto *tl = new QVBoxLayout(tip);
    tl->setContentsMargins(14, 12, 14, 12);
    tl->setSpacing(4);
    auto *tipT = new QLabel(QStringLiteral("Need speed?"), tip);
    tipT->setStyleSheet("color: #F1F5F9; font-size: 12px; font-weight: 800; background: transparent; border: none;");
    auto *tipB = new QLabel(QStringLiteral("Press Ctrl+K to jump to search."), tip);
    tipB->setWordWrap(true);
    tipB->setStyleSheet("color: #94A3B8; font-size: 11px; background: transparent; border: none;");
    tl->addWidget(tipT);
    tl->addWidget(tipB);
    lay->addWidget(tip);
    lay->addSpacing(10);
    auto *foot = new QLabel(QStringLiteral("v1.0.0  •  PRIVATE BUILD"), side);
    foot->setObjectName("SideFoot");
    lay->addWidget(foot);
}

QPushButton *MainWindow::sideItem(const QString &name, const QString &icon) {
    auto *side = findChild<QWidget *>("Sidebar");
    auto *b = new QPushButton(QStringLiteral("  %1").arg(name), side);
    b->setObjectName("SideItem");
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    b->setIconSize(QSize(16, 16));
    b->setMinimumHeight(38);
    b->setProperty("page", name);
    b->setProperty("iconName", icon);
    const auto paint = [b]() {
        const bool on = b->isChecked();
        b->setIcon(Icons::get(b->property("iconName").toString(),
                              on ? QColor("#FFFFFF") : QColor("#7C8DA6"), 16));
    };
    paint();
    connect(b, &QPushButton::toggled, this, [paint]() { paint(); });
    m_nav.push_back(b);
    connect(b, &QPushButton::clicked, this, [this, name]() { goTo(name); });
    side->layout()->addWidget(b);
    return b;
}

bool MainWindow::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::MouseButtonPress) {
        if (o == m_searchPill || (m_searchPill && m_searchPill->isAncestorOf(qobject_cast<QWidget*>(o)))) {
            goTo("Licenses");
            m_licenses->focusSearch();
            return true;
        }
    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);
    Theme::enableDarkFrame(this);
}

void MainWindow::goTo(const QString &name) {
    int idx = m_order.indexOf(name);
    if (idx < 0)
        idx = 0;
    m_stack->setCurrentIndex(idx);
    for (auto *b : m_nav)
        b->setChecked(b->property("page").toString() == m_order[idx]);
    refreshCurrent();
}

void MainWindow::refreshCurrent() {
    switch (m_stack->currentIndex()) {
    case 0: m_dash->refresh(); break;
    case 1: m_licenses->refresh(); break;
    case 2: m_products->refresh(); break;
    case 3: m_users->refresh(); break;
    case 4: m_devices->refresh(); break;
    case 5: m_sessions->refresh(); break;
    case 6: m_logs->refresh(); break;
    default: break;
    }
    updateConnection();
}

void MainWindow::showToast(const QString &msg, int kind) {
    Toast::show(m_stack->currentWidget(), msg, static_cast<Toast::Kind>(kind));
}

void MainWindow::setLoading(bool on) {
    if (on && !m_overlay)
        m_overlay = new LoadingOverlay(m_stack->currentWidget());
    else if (!on && m_overlay) {
        m_overlay->finish();
        m_overlay = nullptr;
    }
}

void MainWindow::updateConnection() {
    m_sys->ping([this](bool online, qint64 ms) {
        Q_UNUSED(ms);
        const bool mock = m_client->baseUrl().isEmpty();
        m_connText->setText(online ? (mock ? QStringLiteral("●  LOCAL READY")
                                                 : QStringLiteral("●  CONNECTED"))
                                   : QStringLiteral("●  OFFLINE"));
        m_connText->setProperty("down", !online);
        m_connText->style()->unpolish(m_connText);
        m_connText->style()->polish(m_connText);
        m_apiStatus->setText(online ? (mock ? "API: LOCAL MODE" : "API: ONLINE") : "API: OFFLINE");
    });
    const auto &c = Config::instance().config();
    if (m_pollTimer) {
        if (c.pollingEnabled)
            m_pollTimer->start(c.pollingIntervalSec * 1000);
        else
            m_pollTimer->stop();
    }
}

} // namespace KeyStar::Ui
