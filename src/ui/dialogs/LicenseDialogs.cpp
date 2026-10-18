#include "ui/dialogs/LicenseDialogs.h"

#include "core/Theme.h"
#include "ui/components/Components.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace KeyStar::Ui {

CreateLicenseDialog::CreateLicenseDialog(const QStringList &products, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Create License");
    setModal(true);
    setMinimumWidth(460);
    Theme::enableDarkFrame(this);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 22, 24, 20);
    lay->setSpacing(12);
    auto *t = new QLabel(QStringLiteral("CREATE LICENSE"), this);
    t->setObjectName("SectionTitle");
    lay->addWidget(t);
    lay->addWidget(secondaryLabel("A new key is generated and copied to your clipboard.", this));

    auto *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignLeft);
    m_product = new QComboBox(this);
    m_product->setObjectName("Field");
    m_product->addItems(products.isEmpty() ? QStringList{"StarLoader"} : products);
    m_duration = new QComboBox(this);
    m_duration->setObjectName("Field");
    m_duration->addItems({"7 Days", "30 Days", "90 Days", "180 Days", "365 Days", "Lifetime"});
    m_duration->setCurrentIndex(1);
    m_qty = new QSpinBox(this);
    m_qty->setObjectName("Field");
    m_qty->setRange(1, 100);
    m_qty->setValue(1);
    m_notes = new QLineEdit(this);
    m_notes->setObjectName("Field");
    m_notes->setPlaceholderText("Optional notes");
    form->addRow("Product", m_product);
    form->addRow("Duration", m_duration);
    form->addRow("Quantity", m_qty);
    form->addRow("Notes", m_notes);
    lay->addLayout(form);

    auto *row = new QHBoxLayout();
    row->addStretch(1);
    auto *cancel = ghostButton("CANCEL", this);
    auto *ok = primaryButton("CREATE LICENSE", this, "plus");
    row->addWidget(cancel);
    row->addWidget(ok);
    lay->addSpacing(4);
    lay->addLayout(row);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
}

QString CreateLicenseDialog::product() const { return m_product->currentText(); }

int CreateLicenseDialog::durationDays() const {
    switch (m_duration->currentIndex()) {
    case 0: return 7;
    case 2: return 90;
    case 3: return 180;
    case 4: return 365;
    case 5: return 36500;
    default: return 30;
    }
}

int CreateLicenseDialog::quantity() const { return m_qty->value(); }
QString CreateLicenseDialog::notes() const { return m_notes->text(); }

static void addMeta(QVBoxLayout *lay, const QString &label, QWidget *value) {
    auto *cap = captionLabel(label.toUpper(), lay->parentWidget());
    lay->addWidget(cap);
    value->setStyleSheet("color: #E6EDF3; font-size: 13px; background: transparent;");
    lay->addWidget(value);
}

LicenseDetailsDialog::LicenseDetailsDialog(const License &lic, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("License Details");
    setModal(true);
    setMinimumSize(480, 600);
    Theme::enableDarkFrame(this);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 22, 24, 20);
    lay->setSpacing(7);
    auto *head = new QHBoxLayout();
    head->setContentsMargins(0, 0, 0, 0);
    auto *t = new QLabel(QStringLiteral("LICENSE"), this);
    t->setObjectName("SectionTitle");
    head->addWidget(t);
    head->addStretch(1);
    auto *copy = ghostButton("COPY", this, "copy");
    connect(copy, &QPushButton::clicked, this, [lic]() {
        QApplication::clipboard()->setText(lic.key);
    });
    head->addWidget(copy);
    lay->addLayout(head);

    auto *key = monoLabel(lic.key, this);
    key->setFont(Theme::monoFont(17, QFont::DemiBold));
    key->setStyleSheet("color: #FFFFFF;");
    lay->addWidget(key);
    lay->addWidget(new StatusBadge(lic.status, this));
    lay->addSpacing(4);
    lay->addWidget(divider(this));
    lay->addSpacing(2);

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(2);
    auto cell = [&](int r, int c, const QString &k, QWidget *v) {
        auto *wrap = new QWidget(this);
        auto *vl = new QVBoxLayout(wrap);
        vl->setContentsMargins(0, 4, 0, 4);
        vl->setSpacing(2);
        vl->addWidget(captionLabel(k.toUpper(), wrap));
        v->setParent(wrap);
        vl->addWidget(v);
        grid->addWidget(wrap, r, c);
    };
    auto val = [&](const QString &s) {
        auto *l = new QLabel(s, this);
        l->setStyleSheet("color: #E6EDF3; font-size: 13px; background: transparent;");
        return l;
    };
    auto mval = [&](const QString &s) {
        auto *l = monoLabel(s, this);
        l->setStyleSheet("color: #C7D1DB; background: transparent;");
        return l;
    };
    cell(0, 0, "Product", val(lic.product));
    cell(0, 1, "User", val(lic.user.isEmpty() ? "—" : lic.user));
    cell(1, 0, "Created", val(fmtDateTime(lic.created)));
    cell(1, 1, "Expires", val(fmtDateTime(lic.expires)));
    cell(2, 0, "HWID", mval(lic.hwid.isEmpty() ? "—" : lic.hwid));
    cell(2, 1, "Devices",
         val(QStringLiteral("%1 / %2").arg(lic.devicesUsed).arg(lic.devicesMax)));
    cell(3, 0, "Last Authentication", val(fmtDateTime(lic.lastAuth)));
    lay->addLayout(grid);
    lay->addSpacing(6);
    lay->addWidget(divider(this));
    lay->addSpacing(4);

    auto *row = new QHBoxLayout();
    auto *ext = primaryButton("EXTEND", this, "clock");
    auto *rst = ghostButton("RESET HWID", this, "key");
    auto *rev = dangerButton("REVOKE", this, "ban");
    row->addWidget(ext);
    row->addWidget(rst);
    row->addWidget(rev);
    lay->addLayout(row);

    const QString k = lic.key;
    connect(ext, &QPushButton::clicked, this, [this, k]() { emit extendRequested(k, 30); });
    connect(rst, &QPushButton::clicked, this, [this, k]() { emit resetHwidRequested(k); });
    connect(rev, &QPushButton::clicked, this, [this, k]() { emit revokeRequested(k); });
}

} // namespace KeyStar::Ui
