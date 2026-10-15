#include "services/SettingsService.h"

#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <QDir>
#endif

SettingsService::SettingsService()
    : m_settings(QStringLiteral("MemoAssistant"), QStringLiteral("MemoAssistant"))
{
}

QPoint SettingsService::floatingBallPosition() const
{
    return m_settings.value(QStringLiteral("floatingBall/position"), QPoint()).toPoint();
}

void SettingsService::setFloatingBallPosition(const QPoint &position)
{
    m_settings.setValue(QStringLiteral("floatingBall/position"), position);
}

int SettingsService::collapseDelayMs() const
{
    return m_settings.value(QStringLiteral("panel/collapseDelayMs"), 700).toInt();
}

bool SettingsService::startAtLogin() const
{
#ifdef Q_OS_WIN
    QSettings registry(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                       QSettings::NativeFormat);
    return registry.contains(QStringLiteral("MemoAssistant"));
#else
    return false;
#endif
}

bool SettingsService::setStartAtLogin(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings registry(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                       QSettings::NativeFormat);
    if (enabled) {
        const QString command = QStringLiteral("\"%1\" --background")
                                    .arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        registry.setValue(QStringLiteral("MemoAssistant"), command);
    } else {
        registry.remove(QStringLiteral("MemoAssistant"));
    }
    registry.sync();
    return registry.status() == QSettings::NoError;
#else
    Q_UNUSED(enabled)
    return false;
#endif
}
