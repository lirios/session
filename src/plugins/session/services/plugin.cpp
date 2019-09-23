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

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QProcessEnvironment>

#include "plugin.h"

#include <sys/types.h>
#include <signal.h>

ServicesPlugin::ServicesPlugin(QObject *parent)
    : Liri::SessionModule(parent)
{
}

Liri::SessionModule::StartupPhase ServicesPlugin::startupPhase() const
{
    return Daemons;
}

bool ServicesPlugin::start(const QStringList &args)
{
    Q_UNUSED(args)

    startService(QStringLiteral("io.liri.Daemon"));
    startService(QStringLiteral("io.liri.Launcher"));

    return true;
}

bool ServicesPlugin::stop()
{
    std::reverse(m_pids.begin(), m_pids.end());

    auto i = m_pids.begin();
    while (i != m_pids.end()) {
        uint pid = (*i);
        ::kill(pid, SIGTERM);
        ::kill(pid, SIGKILL);

        i = m_pids.erase(i);
    }

    return true;
}

void ServicesPlugin::startService(const QString &name)
{
    auto interface = QDBusConnection::sessionBus().interface();

    // Check if the service is already registered
    auto existsReply = interface->isServiceRegistered(name);
    if (existsReply.isValid() && existsReply.value()) {
        auto pidReply = interface->servicePid(name);
        if (pidReply.isValid())
            m_pids.append(pidReply.value());
        return;
    }

    // If not, start the service
    auto reply = interface->startService(name);
    if (reply.isValid()) {
        auto pidReply = interface->servicePid(name);
        if (pidReply.isValid()) {
            m_pids.append(pidReply.value());

            if (name == QStringLiteral("io.liri.Launcher"))
                setEnvironment();
        }
    } else {
        qCWarning(lcSession, "Failed to start \"%s\" D-Bus service: %s",
                  qPrintable(name), qPrintable(reply.error().message()));
    }
}

void ServicesPlugin::setEnvironment()
{
    QStringList whitelist = {
        QStringLiteral("XDG_DATA_HOME"),
        QStringLiteral("XDG_DATA_DIRS"),
        QStringLiteral("XDG_CACHE_HOME"),
        QStringLiteral("XDG_CONFIG_HOME"),
        QStringLiteral("XDG_CONFIG_DIRS"),
        QStringLiteral("DESKTOP_SESSION"),
        QStringLiteral("XDG_MENU_PREFIX"),
        QStringLiteral("XDG_CURRENT_DESKTOP"),
        QStringLiteral("XDG_SESSION_DESKTOP"),
    };

    auto env = QProcessEnvironment::systemEnvironment();
    const auto keys = env.keys();
    for (const auto &key : keys) {
        if (!whitelist.contains(key))
            continue;

        const auto value = env.value(key);

        auto msg = QDBusMessage::createMethodCall(
                    QStringLiteral("io.liri.Launcher"),
                    QStringLiteral("/io/liri/Launcher"),
                    QStringLiteral("io.liri.Launcher"),
                    QStringLiteral("SetEnvironment"));
        msg.setArguments(QVariantList() << key << value);
        QDBusConnection::sessionBus().call(msg);
    }
}
