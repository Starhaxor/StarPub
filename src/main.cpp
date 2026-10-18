// KeyStar Control — private desktop administration panel for KeyStar.
// Entry: theme + config -> login -> main window. No hard-coded secrets.

#include "api/KeyStarClient.h"
#include "core/Config.h"
#include "core/CredentialStore.h"
#include "core/Theme.h"
#include "services/Services.h"
#include "ui/Icons.h"
#include "ui/MainWindow.h"
#include "ui/dialogs/LoginDialog.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("KeyStar");
    app.setApplicationName("KeyStarControl");
    app.setApplicationVersion("1.0.0");

    KeyStar::Config::instance().load();
    KeyStar::Theme::apply();
    app.setWindowIcon(KeyStar::Icons::logo(32));

    KeyStar::Ui::LoginDialog login;
    if (login.exec() != QDialog::Accepted)
        return 0;

    KeyStar::KeyStarClient client;
    client.setBaseUrl(KeyStar::Config::instance().config().apiEndpoint);
    // Session token comes from login only — never from source code.
    client.setToken(KeyStar::CredentialStore::instance().sessionToken());

    KeyStar::LicenseService lics(&client);
    KeyStar::ProductService prods(&client);
    KeyStar::SystemService sys(&client);

    KeyStar::Ui::MainWindow w(&client, &lics, &prods, &sys);
    w.show();
    return app.exec();
}
