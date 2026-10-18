#pragma once

#include <QObject>
#include <QString>

namespace KeyStar {

// Spec Section 37 — Settings model. Single Carbon Dark theme for now.
// Persisted via QSettings (no secrets here — see CredentialStore).
struct AppConfig {
    QString apiEndpoint;      // empty = mock/local mode
    int requestTimeoutMs = 15000;
    QString startupPage = "Dashboard";
    bool notificationsEnabled = true;
    bool pollingEnabled = false;
    int pollingIntervalSec = 45;
    bool developerMode = false;
    bool debugLogging = false;
    int sessionTimeoutMin = 30;
};

class Config : public QObject {
    Q_OBJECT
public:
    static Config &instance();
    void load();
    void save();

    AppConfig &config() { return m_cfg; }
    const AppConfig &config() const { return m_cfg; }

signals:
    void changed();

private:
    Config() = default;
    AppConfig m_cfg;
};

} // namespace KeyStar
