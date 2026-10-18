#include "ui/pages/LicensesPage.h"

#include "core/Theme.h"
#include "models/Models.h"
#include "ui/components/Components.h"
#include "ui/dialogs/LicenseDialogs.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

namespace KeyStar::Ui {

LicensesPage::LicensesPage(LicenseService *lics, ProductService *prods, QWidget *parent)
    : QWidget(parent), m_lics(lics), m_prods(prods) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(14);

    auto *head = new QHBoxLayout();
    head->setContentsMargins(0, 0, 0, 0);
    auto *title = new QLabel(QStringLiteral("Licenses"), this);
    title->setObjectName("PageTitle");
    head->addWidget(title);
    head->addStretch(1);
    auto *createBtn = primaryButton("NEW LICENSE", this, "plus");
    createBtn->setObjectName("Primary");
    head->addWidget(createBtn);
    lay->addLayout(head);

    auto *bar = new QHBoxLayout();
    bar->setSpacing(10);
    m_search = new SearchBar("Search key, user, HWID, product, status…   (Ctrl+K)", this);
    m_search->setMinimumWidth(300);
    m_statusBox = new QComboBox(this);
    m_statusBox->setObjectName("Field");
    m_statusBox->setFixedWidth(130);
    m_statusBox->addItems({"ALL", "ACTIVE", "EXPIRED", "REVOKED", "PENDING"});
    m_productBox = new QComboBox(this);
    m_productBox->setObjectName("Field");
    m_productBox->setFixedWidth(150);
    m_productBox->addItem("ALL");
    auto *refreshBtn = ghostButton("REFRESH", this, "refresh");
    bar->addWidget(m_search, 1);
    bar->addWidget(m_statusBox);
    bar->addWidget(m_productBox);
    bar->addWidget(refreshBtn);
    lay->addLayout(bar);

    m_listWrap = new QWidget(this);
    auto *ll = new QVBoxLayout(m_listWrap);
    ll->setContentsMargins(0, 0, 0, 0);
    ll->setSpacing(10);

    m_table = new QTableWidget(this);
    m_table->setObjectName("DataTable");
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels(
        {"KEY", "PRODUCT", "STATUS", "CREATED", "EXPIRES", "USER", "DEVICE"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setWordWrap(false);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(38);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int c = 1; c < 7; ++c)
        m_table->horizontalHeader()->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->setSortingEnabled(true);
    ll->addWidget(m_table, 1);

    auto *acts = new QHBoxLayout();
    acts->setSpacing(6);
    m_pageLabel = new QLabel(this);
    m_pageLabel->setObjectName("Secondary");
    auto *prev = iconButton("chevron-left", "Previous page", this);
    auto *next = iconButton("chevron-right", "Next page", this);
    auto *view = iconButton("eye", "View license details", this);
    auto *ext = iconButton("clock", "Extend 30 days", this);
    auto *rst = iconButton("key", "Reset HWID", this);
    auto *rev = iconButton("ban", "Revoke license", this, true);
    auto *del = iconButton("trash", "Delete license", this, true);
    acts->addWidget(m_pageLabel);
    acts->addStretch(1);
    acts->addWidget(prev);
    acts->addWidget(next);
    acts->addSpacing(8);
    acts->addWidget(view);
    acts->addWidget(ext);
    acts->addWidget(rst);
    acts->addWidget(rev);
    acts->addWidget(del);
    ll->addLayout(acts);
    lay->addWidget(m_listWrap, 1);

    auto currentKey = [this]() -> QString {
        const int r = m_table->currentRow();
        if (r < 0)
            return {};
        auto *it = m_table->item(r, 0);
        return it ? it->data(Qt::UserRole).toString() : QString();
    };

    connect(m_search, &SearchBar::debounced, this, [this](const QString &t) {
        m_filter.text = t;
        m_filter.page = 0;
        reload();
    });
    connect(m_statusBox, &QComboBox::currentTextChanged, this, [this](const QString &s) {
        m_filter.status = s;
        m_filter.page = 0;
        reload();
    });
    connect(m_productBox, &QComboBox::currentTextChanged, this, [this](const QString &s) {
        m_filter.product = s;
        m_filter.page = 0;
        reload();
    });
    connect(refreshBtn, &QPushButton::clicked, this, &LicensesPage::refresh);
    connect(createBtn, &QPushButton::clicked, this, &LicensesPage::openCreate);
    connect(prev, &QPushButton::clicked, this, [this]() {
        if (m_filter.page > 0) {
            --m_filter.page;
            reload();
        }
    });
    connect(next, &QPushButton::clicked, this, [this]() {
        ++m_filter.page;
        reload();
    });
    connect(view, &QPushButton::clicked, this, [currentKey, this]() {
        if (!currentKey().isEmpty())
            openDetails(currentKey());
    });
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int r, int) {
        auto *it = m_table->item(r, 0);
        if (it)
            openDetails(it->data(Qt::UserRole).toString());
    });
    connect(ext, &QPushButton::clicked, this, [currentKey, this]() {
        if (!currentKey().isEmpty())
            doExtend(currentKey());
    });
    connect(rst, &QPushButton::clicked, this, [currentKey, this]() {
        if (!currentKey().isEmpty())
            doResetHwid(currentKey());
    });
    connect(rev, &QPushButton::clicked, this, [currentKey, this]() {
        if (!currentKey().isEmpty())
            doRevoke(currentKey());
    });
    connect(del, &QPushButton::clicked, this, [currentKey, this]() {
        if (!currentKey().isEmpty())
            doDelete(currentKey());
    });

    m_prods->list([this](const QVector<Product> &ps) {
        for (const auto &p : ps) {
            m_productNames << p.name;
            if (m_productBox->findText(p.name) < 0)
                m_productBox->addItem(p.name);
        }
    });

    refresh();
}

void LicensesPage::refresh() {
    m_filter.page = 0;
    reload();
}

void LicensesPage::focusSearch() {
    m_search->focusSearch();
}

void LicensesPage::setProductFilter(const QString &product) {
    const int i = m_productBox->findText(product);
    m_productBox->setCurrentIndex(i < 0 ? 0 : i);
    // currentTextChanged fires reload; force in case index unchanged
    m_filter.product = i < 0 ? "ALL" : product;
    m_filter.page = 0;
    reload();
}

void LicensesPage::reload() {
    emit loading(true);
    m_lics->list(m_filter, [this](const LicensePage &page) {
        emit loading(false);
        fillTable(page);
    });
}

void LicensesPage::fillTable(const LicensePage &page) {
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);
    if (page.total == 0) {
        m_table->setSortingEnabled(true);
        m_listWrap->hide();
        if (!m_emptyWrap) {
            auto *btn = primaryButton("CREATE LICENSE", nullptr, "plus");
            m_emptyWrap = emptyState("No licenses",
                "There are no licenses matching your search.", btn, "key");
            static_cast<QVBoxLayout *>(layout())->addWidget(m_emptyWrap, 1);
            connect(btn, &QPushButton::clicked, this, &LicensesPage::openCreate);
        }
        m_emptyWrap->show();
        return;
    }
    m_listWrap->show();
    if (m_emptyWrap)
        m_emptyWrap->hide();

    m_table->setRowCount(page.rows.size());
    for (int r = 0; r < page.rows.size(); ++r) {
        const auto &l = page.rows[r];
        auto *k = new QTableWidgetItem(l.key);
        k->setFont(Theme::monoFont(12, QFont::DemiBold));
        k->setData(Qt::UserRole, l.key);
        m_table->setItem(r, 0, k);
        m_table->setItem(r, 1, new QTableWidgetItem(l.product));
        auto *st = new QTableWidgetItem(QStringLiteral("● ") + l.status);
        st->setForeground(Theme::statusColor(l.status));
        st->setFont(Theme::uiFont(12, QFont::DemiBold));
        m_table->setItem(r, 2, st);
        m_table->setItem(r, 3, new QTableWidgetItem(fmtDate(l.created)));
        auto *exp = new QTableWidgetItem(fmtDate(l.expires));
        if (l.isExpired() && l.status == "ACTIVE")
            exp->setForeground(Theme::statusColor("EXPIRED"));
        m_table->setItem(r, 4, exp);
        m_table->setItem(r, 5, new QTableWidgetItem(l.user.isEmpty() ? "—" : l.user));
        auto *hw = new QTableWidgetItem(l.hwid.isEmpty() ? "—" : l.hwid);
        hw->setFont(Theme::monoFont(12));
        hw->setForeground(Colors::TextSecondary);
        m_table->setItem(r, 6, hw);
    }
    m_table->setSortingEnabled(true);
    if (m_table->rowCount() > 0)
        m_table->selectRow(0);
    const int pages = qMax(1, (page.total + m_filter.pageSize - 1) / m_filter.pageSize);
    m_pageLabel->setText(
        QStringLiteral("%1 licenses  •  page %2 / %3")
            .arg(page.total)
            .arg(m_filter.page + 1)
            .arg(pages));
}

void LicensesPage::openCreate() {
    CreateLicenseDialog d(m_productNames, this);
    if (d.exec() != QDialog::Accepted)
        return;
    emit loading(true);
    m_lics->create(d.product(), d.durationDays(), d.quantity(), d.notes(),
        [this](bool ok, const QStringList &keys, const QString &msg) {
            emit loading(false);
            if (ok) {
                QApplication::clipboard()->setText(keys.join("\n"));
                emit toast(msg + " — key copied to clipboard", 0);
            } else {
                emit toast("✕ " + msg, 2);
            }
            reload();
        });
}

void LicensesPage::openDetails(const QString &key) {
    LicenseFilter f;
    f.text = key;
    f.pageSize = 5;
    emit loading(true);
    m_lics->list(f, [this, key](const LicensePage &p) {
        emit loading(false);
        if (p.rows.isEmpty()) {
            emit toast("✕ License not found", 2);
            return;
        }
        LicenseDetailsDialog d(p.rows.first(), this);
        connect(&d, &LicenseDetailsDialog::extendRequested, this,
                [this](const QString &k, int) { doExtend(k); });
        connect(&d, &LicenseDetailsDialog::resetHwidRequested, this,
                [this](const QString &k) { doResetHwid(k); });
        connect(&d, &LicenseDetailsDialog::revokeRequested, this,
                [this](const QString &k) { doRevoke(k); });
        d.exec();
        reload();
    });
}

void LicensesPage::doExtend(const QString &key) {
    emit loading(true);
    m_lics->extend(key, 30, [this](bool ok, const License &, const QString &msg) {
        emit loading(false);
        emit toast(ok ? QStringLiteral("✓ ") + msg : QStringLiteral("✕ ") + msg,
                   ok ? 0 : 2);
        reload();
    });
}

void LicensesPage::doResetHwid(const QString &key) {
    if (!confirmAction(this, "Reset HWID",
            QStringLiteral("Reset HWID binding for:\n%1").arg(key),
            "RESET HWID"))
        return;
    emit loading(true);
    m_lics->resetHwid(key, [this](bool ok, const QString &msg) {
        emit loading(false);
        emit toast(ok ? QStringLiteral("✓ ") + msg : QStringLiteral("✕ ") + msg,
                   ok ? 0 : 2);
        reload();
    });
}

void LicensesPage::doRevoke(const QString &key) {
    if (!confirmAction(this, "Revoke License",
            QStringLiteral("Are you sure you want to revoke:\n%1\n\n"
                           "This action may prevent the client from authenticating.").arg(key),
            "REVOKE", true))
        return;
    emit loading(true);
    m_lics->revoke(key, [this](bool ok, const QString &msg) {
        emit loading(false);
        emit toast(ok ? QStringLiteral("✓ ") + msg : QStringLiteral("✕ ") + msg,
                   ok ? 0 : 2);
        reload();
    });
}

void LicensesPage::doDelete(const QString &key) {
    if (!confirmDelete(this, key))
        return;
    emit loading(true);
    m_lics->remove(key, [this](bool ok, const QString &msg) {
        emit loading(false);
        emit toast(ok ? QStringLiteral("✓ ") + msg : QStringLiteral("✕ ") + msg,
                   ok ? 0 : 2);
        reload();
    });
}

} // namespace KeyStar::Ui
