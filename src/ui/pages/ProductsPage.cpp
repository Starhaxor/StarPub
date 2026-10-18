#include "ui/pages/ProductsPage.h"

#include "models/Models.h"
#include "core/Theme.h"
#include "services/Services.h"
#include "ui/Icons.h"
#include "ui/components/Components.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace KeyStar::Ui {

ProductsPage::ProductsPage(ProductService *prods, QWidget *parent)
    : QWidget(parent), m_prods(prods) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(14);
    auto *head = new QHBoxLayout();
    head->setContentsMargins(0, 0, 0, 0);
    auto *title = new QLabel(QStringLiteral("Products"), this);
    title->setObjectName("PageTitle");
    head->addWidget(title);
    head->addStretch(1);
    m_count = captionLabel("", this);
    head->addWidget(m_count);
    lay->addLayout(head);
    m_cards = new QWidget(this);
    m_cards->setLayout(new QGridLayout());
    static_cast<QGridLayout *>(m_cards->layout())->setSpacing(14);
    lay->addWidget(m_cards);
    lay->addStretch(1);
    refresh();
}

void ProductsPage::refresh() {
    emit loading(true);
    m_prods->list([this](const QVector<Product> &ps) {
        emit loading(false);
        QLayoutItem *it;
        while ((it = m_cards->layout()->takeAt(0)) != nullptr) {
            delete it->widget();
            delete it;
        }
        m_count->setText(QStringLiteral("%1 PRODUCTS").arg(ps.size()));
        auto *grid = static_cast<QGridLayout *>(m_cards->layout());
        int i = 0;
        for (const auto &p : ps) {
            auto *card = new QFrame(m_cards);
            card->setObjectName("CardHover");
            card->setMinimumWidth(250);
            card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            auto *cl = new QVBoxLayout(card);
            cl->setContentsMargins(18, 16, 18, 14);
            cl->setSpacing(10);

            auto *top = new QHBoxLayout();
            top->setSpacing(12);
            auto *tile = new QLabel(card);
            tile->setFixedSize(40, 40);
            tile->setPixmap(Icons::pixmap("package", Colors::Primary, 22));
            tile->setAlignment(Qt::AlignCenter);
            tile->setStyleSheet("background: #0E2B33; border: 1px solid #16424D; border-radius: 8px;");
            auto *nt = new QVBoxLayout();
            nt->setSpacing(1);
            auto *name = new QLabel(p.name, card);
            name->setObjectName("SectionTitle");
            auto *ident = monoLabel(p.identifier, card);
            ident->setStyleSheet("color: #5E6A75;");
            nt->addWidget(name);
            nt->addWidget(ident);
            top->addWidget(tile);
            top->addLayout(nt, 1);
            top->addWidget(new StatusBadge(p.status, card));
            cl->addLayout(top);
            cl->addWidget(divider(card));

            auto *stats = new QGridLayout();
            stats->setContentsMargins(0, 0, 0, 0);
            stats->setHorizontalSpacing(18);
            stats->setVerticalSpacing(8);
            auto stat = [&](int r, int c, const QString &k, const QString &v, bool mono = false) {
                auto *kl = captionLabel(k.toUpper(), card);
                QWidget *vl = mono ? static_cast<QWidget *>(monoLabel(v, card))
                                   : static_cast<QWidget *>(new QLabel(v, card));
                if (!mono)
                    vl->setStyleSheet("color: #E6EDF3; font-size: 13px;");
                else
                    vl->setStyleSheet("color: #C7D1DB;");
                stats->addWidget(kl, r * 2, c);
                stats->addWidget(vl, r * 2 + 1, c);
            };
            stat(0, 0, "Licenses", QString::number(p.licenseCount));
            stat(0, 1, "Version", "v" + p.version, true);
            stat(1, 0, "Created", fmtDate(p.created));
            stat(1, 1, "Status", p.status.toLower());
            cl->addLayout(stats);

            auto *foot = new QHBoxLayout();
            foot->setContentsMargins(0, 0, 0, 0);
            foot->addStretch(1);
            auto *link = linkButton("VIEW LICENSES  →", card);
            const QString pname = p.name;
            connect(link, &QPushButton::clicked, this, [this, pname]() {
                emit viewLicenses(pname);
            });
            foot->addWidget(link);
            cl->addLayout(foot);

            grid->addWidget(card, i / 3, i % 3);
            ++i;
        }
    });
}

} // namespace KeyStar::Ui
