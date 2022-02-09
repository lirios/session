/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QDBusConnection>
#include <QDBusError>

#include "screensaver.h"
#include "session.h"
#include "backends/sessionbackend.h"

ScreenSaver::ScreenSaver(QObject *parent)
    : QObject(parent)
{
    connect(SessionBackend::instance(), &SessionBackend::sessionLocked,
            this, &ScreenSaver::handleLock);
    connect(SessionBackend::instance(), &SessionBackend::sessionUnlocked,
            this, &ScreenSaver::handleUnlock);
    connect(SessionBackend::instance(), &SessionBackend::inhibited,
            this, &ScreenSaver::handleInhibited);
}

ScreenSaver::~ScreenSaver()
{
    // Unregister D-Bus object
    auto bus = QDBusConnection::sessionBus();
    bus.unregisterObject(objectPath);
    bus.unregisterService(serviceName);
}

bool ScreenSaver::registerWithDBus()
{
    auto bus = QDBusConnection::sessionBus();

    // Register D-Bus service
    if (!bus.registerService(serviceName)) {
        qCWarning(lcSession,
                  "Failed to register D-Bus service \"%s\": %s",
                  qPrintable(serviceName),
                  qPrintable(bus.lastError().message()));
        return false;
    }

    // Register D-Bus object
    if (!bus.registerObject(objectPath, serviceName, this,
                            QDBusConnection::ExportScriptableContents)) {
        qCWarning(lcSession,
                  "Failed to register \"%s\" D-Bus object for %s: %s",
                  qPrintable(objectPath),
                  qPrintable(serviceName),
                  qPrintable(bus.lastError().message()));
        bus.unregisterService(serviceName);
        return false;
    }

    return true;
}

bool ScreenSaver::GetActive()
{
    return m_active;
}

bool ScreenSaver::SetActive(bool state)
{
    // SetActive activates the screensaver, hence we lock the session
    if (state) {
        Lock();
        return true;
    }

    return false;
}

uint ScreenSaver::GetActiveTime()
{
    if (m_elapsedTimer.isValid()) {
        qint64 elapsed = m_elapsedTimer.elapsed();
        return elapsed > 0 ? uint(elapsed) : 0;
    }
    return 0;
}

uint ScreenSaver::GetSessionIdleTime()
{
    // TODO:
    return 0;
}

void ScreenSaver::SimulateUserActivity()
{
    // TODO:
}

uint ScreenSaver::Inhibit(const QString &appName, const QString &reason)
{
    static uint cookieSeed = 0;
    uint newCookie = cookieSeed++;

    m_inhibit[newCookie] = InhibitEntry{ appName, reason };

    SessionBackend::instance()->inhibitIdle(appName, reason);

    return newCookie;
}

void ScreenSaver::UnInhibit(uint cookie)
{
    int fd = m_inhibitFd.value(cookie, -1);

    if (fd != -1) {
        SessionBackend::instance()->uninhibitIdle(fd);

        m_inhibit.remove(cookie);
        m_inhibitFd.remove(cookie);
    }
}

void ScreenSaver::Lock()
{
    SessionBackend::instance()->lockSession();
}

uint ScreenSaver::Throttle(const QString &appName, const QString &reason)
{
    // TODO:
    Q_UNUSED(appName)
    Q_UNUSED(reason)
    return 0;
}

void ScreenSaver::UnThrottle(uint cookie)
{
    // TODO:
    Q_UNUSED(cookie)
}

void ScreenSaver::handleLock()
{
    m_active = true;
    m_elapsedTimer.restart();
    emit ActiveChanged(m_active);
}

void ScreenSaver::handleUnlock()
{
    m_active = false;
    m_elapsedTimer.invalidate();
    emit ActiveChanged(m_active);
}

void ScreenSaver::handleInhibited(const QString &who, const QString &why, int fd)
{
    const auto keys = m_inhibit.keys();
    for (auto cookie : keys) {
        if (m_inhibit[cookie].who == who && m_inhibit[cookie].why == why) {
            m_inhibitFd[cookie] = fd;
            break;
        }
    }
}
