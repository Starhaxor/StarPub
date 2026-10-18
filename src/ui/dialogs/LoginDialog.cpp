#include "ui/dialogs/LoginDialog.h"

#include "core/Config.h"
#include "core/CredentialStore.h"
#include "core/Theme.h"
#include "ui/Icons.h"
#include "ui/components/Components.h"

#include <QApplication>
#include <QDateTime>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace KeyStar::Ui {

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("KeyStar Control — Sign In");
    setWindowIcon(Icons::logo(32));
    setModal(true);
    setFixedSize(440, 640);
    setObjectName("AppRoot");
    Theme::enableDarkFrame(this);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setAlignment(Qt::AlignCenter);

    auto *card = new QFrame(this);
    card->setObjectName("CardElevated");
    card->setFixedWidth(372);
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(36, 28, 36, 28);
    cl->setSpacing(10);
    cl->setAlignment(Qt::AlignCenter);

    // gradient accent line on top
    auto *accent = new QWidget(card);
    accent->setObjectName("AccentLine");
    accent->setFixedHeight(4);
    accent->setMinimumWidth(120);
    auto *accentRow = new QHBoxLayout();
    accentRow->setAlignment(Qt::AlignCenter);
    accentRow->addWidget(accent);
    cl->addLayout(accentRow);
    cl->addSpacing(4);

    auto *logoBox = new QWidget(card);
    logoBox->setObjectName("SideLogoBox");
    logoBox->setFixedSize(64, 64);
    auto *lbox = new QHBoxLayout(logoBox);
    lbox->setContentsMargins(0, 0, 0, 0);
    lbox->setAlignment(Qt::AlignCenter);
    auto *logo = new QLabel(logoBox);
    logo->setPixmap(Icons::logo(40).pixmap(40, 40));
    logo->setFixedSize(40, 40);
    logo->setStyleSheet("background: transparent; border: none;");
    lbox->addWidget(logo);
    auto *logoRow = new QHBoxLayout();
    logoRow->setAlignment(Qt::AlignCenter);
    logoRow->addWidget(logoBox);
    cl->addLayout(logoRow);

    auto *brand = new QLabel(QStringLiteral("KEYSTAR"), card);
    brand->setAlignment(Qt::AlignCenter);
    brand->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 800; background: transparent;");
    auto *sub = new QLabel(QStringLiteral("CONTROL CENTER"), card);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color: #22D3EE; font-size: 11px; font-weight: 800; background: transparent;");
    auto *cap = new QLabel(QStringLiteral("PRIVATE ADMIN CONSOLE  •  v1.0.0"), card);
    cap->setAlignment(Qt::AlignCenter);
    cap->setStyleSheet("color: #5B6B82; font-size: 10px; font-weight: 700; background: transparent;");
    cl->addWidget(brand);
    cl->addWidget(sub);
    cl->addWidget(cap);
    cl->addSpacing(10);

    m_userEdit = new IconField("user", "Administrator", card);
    m_userEdit->setText(CredentialStore::instance().lastUsername());
    m_passEdit = new IconField("lock", "Password", card);
    m_passEdit->setEchoMode(QLineEdit::Password);
    cl->addWidget(m_userEdit);
    cl->addWidget(m_passEdit);

    m_error = new QLabel(card);
    m_error->setStyleSheet("color: #FB7185; font-size: 12px; font-weight: 600; background: #2A1218; "
                           "border: 1px solid #5B2230; border-radius: 8px; padding: 8px;");
    m_error->setAlignment(Qt::AlignCenter);
    m_error->setWordWrap(true);
    m_error->hide();
    cl->addWidget(m_error);

    m_signIn = primaryButton("SIGN IN  →", card);
    m_signIn->setDefault(true);
    m_signIn->setMinimumHeight(44);
    cl->addSpacing(2);
    cl->addWidget(m_signIn);

    cl->addSpacing(6);
    m_apiState = new QLabel(card);
    m_apiState->setAlignment(Qt::AlignCenter);
    const bool mock = Config::instance().config().apiEndpoint.isEmpty();
    m_apiState->setText(mock ? QStringLiteral("●  LOCAL MODE READY")
                             : QStringLiteral("●  API READY"));
    m_apiState->setStyleSheet(
        "color: #34D399; font-size: 11px; font-weight: 800; background: #0A2621; "
        "border: 1px solid #134E3E; border-radius: 12px; padding: 6px 12px;");
    cl->addWidget(m_apiState);

    lay->addWidget(card);

    connect(m_signIn, &QPushButton::clicked, this, &LoginDialog::tryLogin);
    connect(m_passEdit->edit(), &QLineEdit::returnPressed, this, &LoginDialog::tryLogin);
    connect(m_userEdit->edit(), &QLineEdit::returnPressed, this, &LoginDialog::tryLogin);
}

void LoginDialog::tryLogin() {
    const QString u = m_userEdit->text().trimmed();
    const QString p = m_passEdit->text();
    // MVP auth: local check. Real backend verifies via
    // KeyStarClient::post("/auth/login") — Phase 2 wiring.
    if (u.isEmpty() || p.isEmpty()) {
        m_error->setText(QStringLiteral("✕  Authentication failed — enter credentials"));
        m_error->show();
        return;
    }
    m_user = u;
    CredentialStore::instance().setSessionToken(
        QStringLiteral("local-%1-%2").arg(u).arg(QDateTime::currentSecsSinceEpoch()));
    CredentialStore::instance().setLastUsername(u);
    accept();
}

} // namespace KeyStar::Ui
