#include "core/CredentialStore.h"

#include <QSettings>

namespace KeyStar {

CredentialStore &CredentialStore::instance() {
    static CredentialStore s;
    return s;
}

void CredentialStore::setLastUsername(const QString &u) {
    QSettings s("KeyStar", "KeyStarControl");
    s.setValue("auth/lastUser", u);
}

QString CredentialStore::lastUsername() const {
    QSettings s("KeyStar", "KeyStarControl");
    return s.value("auth/lastUser", "Administrator").toString();
}

// TODO(security-phase2): persist API secret via Windows Credential Manager
// (advapi32 CredWriteW/CredReadW, target "KeyStarControl/api-secret") instead
// of QSettings, and DPAPI-protect any local cache. Never log the secret;
// Developer Mode may show endpoint/status/latency but must redact tokens.

} // namespace KeyStar
