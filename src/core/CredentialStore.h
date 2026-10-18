#pragma once

#include <QObject>
#include <QString>

namespace KeyStar {

// Spec Section 19 — Security.
// NEVER hard-code API secrets. This store keeps tokens in-memory during the
// session and persists only an obfuscated (NOT secure) placeholder via
// QSettings. Production hardening: move to Windows Credential Manager
// (CredWrite/CredRead) — see TODO in CredentialStore.cpp.
class CredentialStore : public QObject {
    Q_OBJECT
public:
    static CredentialStore &instance();

    void setSessionToken(const QString &token) { m_token = token; }
    QString sessionToken() const { return m_token; }
    void clearSession() { m_token.clear(); }
    bool hasSession() const { return !m_token.isEmpty(); }

    // Non-secret metadata only (e.g. last username, endpoint). No plaintext secrets.
    void setLastUsername(const QString &u);
    QString lastUsername() const;

private:
    CredentialStore() = default;
    QString m_token;
};

} // namespace KeyStar
