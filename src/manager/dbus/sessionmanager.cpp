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

#include "backends/sessionbackend.h"
#include "session.h"
#include "sessionmanager.h"
#include "sessionmanageradaptor.h"

const QString serviceName = QStringLiteral("io.liri.SessionManager");
const QString objectPath = QStringLiteral("/io/liri/SessionManager");

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_session(qobject_cast<Session *>(parent))
{
    new SessionManagerAdaptor(this);

    connect(SessionBackend::instance(), &SessionBackend::sessionLocked,
            this, &SessionManager::Locked);
    connect(SessionBackend::instance(), &SessionBackend::sessionUnlocked,
            this, &SessionManager::Unlocked);
}

SessionManager::~SessionManager()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    bus.unregisterObject(objectPath);
    bus.unregisterService(serviceName);
}

bool SessionManager::registerWithDBus()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.registerService(serviceName)) {
        qCWarning(lcSession, "Couldn't register %s D-Bus service: %s",
                  qPrintable(serviceName),
                  qPrintable(bus.lastError().message()));
        return false;
    }

    if (!bus.registerObject(objectPath, this)) {
        qCWarning(lcSession, "Couldn't register %s D-Bus object: %s",
                  qPrintable(objectPath),
                  qPrintable(bus.lastError().message()));
        return false;
    }

    return true;
}

void SessionManager::SetEnvironment(const QString &key, const QString &value)
{
    if (m_session)
        m_session->setEnvironment(key, value);
}

void SessionManager::UnsetEnvironment(const QString &key)
{
    if (m_session)
        m_session->unsetEnvironment(key);
}

void SessionManager::SetIdle(bool idle)
{
    SessionBackend::instance()->setIdle(idle);
}

void SessionManager::Lock()
{
    SessionBackend::instance()->lockSession();
}

void SessionManager::Unlock()
{
    SessionBackend::instance()->unlockSession();
}

void SessionManager::Logout()
{
    if (m_session)
        m_session->shutdown();
}
