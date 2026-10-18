#include "core/Config.h"

#include <QSettings>

namespace KeyStar {

Config &Config::instance() {
    static Config s;
    return s;
}

void Config::load() {
    QSettings s("KeyStar", "KeyStarControl");
    m_cfg.apiEndpoint = s.value("api/endpoint", "").toString();
    m_cfg.requestTimeoutMs = s.value("api/timeoutMs", 15000).toInt();
    m_cfg.startupPage = s.value("general/startupPage", "Dashboard").toString();
    m_cfg.notificationsEnabled = s.value("general/notifications", true).toBool();
    m_cfg.pollingEnabled = s.value("general/polling", false).toBool();
    m_cfg.pollingIntervalSec = s.value("general/pollingSec", 45).toInt();
    m_cfg.developerMode = s.value("developer/enabled", false).toBool();
    m_cfg.debugLogging = s.value("developer/debugLog", false).toBool();
    m_cfg.sessionTimeoutMin = s.value("security/sessionMin", 30).toInt();
}

void Config::save() {
    QSettings s("KeyStar", "KeyStarControl");
    s.setValue("api/endpoint", m_cfg.apiEndpoint);
    s.setValue("api/timeoutMs", m_cfg.requestTimeoutMs);
    s.setValue("general/startupPage", m_cfg.startupPage);
    s.setValue("general/notifications", m_cfg.notificationsEnabled);
    s.setValue("general/polling", m_cfg.pollingEnabled);
    s.setValue("general/pollingSec", m_cfg.pollingIntervalSec);
    s.setValue("developer/enabled", m_cfg.developerMode);
    s.setValue("developer/debugLog", m_cfg.debugLogging);
    s.setValue("security/sessionMin", m_cfg.sessionTimeoutMin);
    emit changed();
}

} // namespace KeyStar
