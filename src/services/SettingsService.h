#pragma once

#include <QPoint>
#include <QSettings>

class SettingsService
{
public:
    SettingsService();

    QPoint floatingBallPosition() const;
    void setFloatingBallPosition(const QPoint &position);
    int collapseDelayMs() const;
    bool startAtLogin() const;
    bool setStartAtLogin(bool enabled);

private:
    QSettings m_settings;
};
