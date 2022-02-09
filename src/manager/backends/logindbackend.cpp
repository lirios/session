/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
#include <QDBusConnectionInterface>

#include <LiriAuroraLogind/Logind>

#include "logindbackend.h"

#include <unistd.h>

using namespace Aurora::PlatformSupport;

LogindBackend::LogindBackend()
    : SessionBackend()
{
    Logind *logind = Logind::instance();

    connect(logind, &Logind::connectedChanged,
            this, &LogindBackend::handleConnectedChanged);
    connect(logind, &Logind::inhibited,
            this, &LogindBackend::handleInhibited);
    connect(logind, &Logind::uninhibited,
            this, &LogindBackend::handleUninhibited);
    connect(logind, &Logind::prepareForSleep,
            this, &LogindBackend::prepareForSleep);
    connect(logind, &Logind::prepareForShutdown,
            this, &LogindBackend::prepareForShutdown);
    connect(logind, &Logind::lockSessionRequested,
            this, &LogindBackend::handleSessionLocked);
    connect(logind, &Logind::unlockSessionRequested,
            this, &LogindBackend::handleSessionUnlocked);

    setupInhibitors();
}

LogindBackend::~LogindBackend()
{
    for (const auto it = m_fds.cbegin(); it != m_fds.cend(); )
        Logind::instance()->uninhibit(it.value());
}

QString LogindBackend::name() const
{
    return QStringLiteral("logind");
}

void LogindBackend::setIdle(bool value)
{
    Logind::instance()->setIdleHint(value);
}

void LogindBackend::inhibitIdle(const QString &who, const QString &why)
{
    Logind::instance()->inhibit(who, why,
                                Logind::InhibitIdle,
                                Logind::Block);
}

void LogindBackend::uninhibitIdle(int fd)
{
    Logind::instance()->uninhibit(fd);
}

void LogindBackend::lockSession()
{
    Logind::instance()->lockSession();
}

void LogindBackend::unlockSession()
{
    Logind::instance()->unlockSession();
}

void LogindBackend::switchToVt(quint32 vt)
{
    Logind::instance()->switchTo(vt);
}

bool LogindBackend::exists()
{
    return QDBusConnection::systemBus().interface()->isServiceRegistered(QStringLiteral("org.freedesktop.login1"));
}

void LogindBackend::setupInhibitors()
{
    Logind *logind = Logind::instance();
    logind->inhibit(
                QStringLiteral("Liri/PowerButton"),
                QStringLiteral("Liri handles the power button itself"),
                Logind::InhibitPowerKey | Logind::InhibitSuspendKey | Logind::InhibitHibernateKey,
                Logind::Block);
    logind->inhibit(
                QStringLiteral("Liri/Sleep"),
                QStringLiteral("Liri needs to logout before shutdown and lock the screen before sleep"),
                Logind::InhibitShutdown | Logind::InhibitSleep,
                Logind::Delay);
    logind->inhibit(
                QStringLiteral("Liri/Lid"),
                QStringLiteral("Liri wants to handle when the lid is closed"),
                Logind::InhibitLidSwitch,
                Logind::Block);
}

void LogindBackend::handleConnectedChanged(bool connected)
{
    // Unset idle hint at startup so that the login manager
    // will report the flag correctly
    if (connected)
        setIdle(false);
}

void LogindBackend::handleInhibited(const QString &who, const QString &why, int fd)
{
    Q_UNUSED(why);

    m_fds[who] = fd;

    Q_EMIT inhibited(who, why, fd);
}

void LogindBackend::handleUninhibited(int fd)
{
    const auto who = m_fds.key(fd);
    if (!who.isEmpty())
        m_fds.remove(who);
}

void LogindBackend::prepareForSleep(bool arg)
{
    // Lock immediately when the system is going to sleep
    if (arg)
        emit sessionLocked();
}

void LogindBackend::prepareForShutdown(bool arg)
{
    // Bring down the session before the system is shut down
    if (arg)
        emit shutdownRequested();
}

void LogindBackend::handleSessionLocked()
{
    // Uninhibit everything when the session is locked
    for (const auto it = m_fds.cbegin(); it != m_fds.cend(); )
        Logind::instance()->uninhibit(it.value());

    emit sessionLocked();
}

void LogindBackend::handleSessionUnlocked()
{
    // Inhibit again when the session is unlocked
    setupInhibitors();

    emit sessionUnlocked();
}
