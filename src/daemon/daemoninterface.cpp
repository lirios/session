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

#include "daemon.h"
#include "daemoninterface.h"
#include "daemonadaptor.h"

#define SERVICE_NAME QStringLiteral("io.liri.Daemon")
#define OBJECT_PATH QStringLiteral("/io/liri/Daemon")

DaemonInterface::DaemonInterface(QObject *parent)
    : QObject(parent)
{
    new DaemonAdaptor(this);
}

DaemonInterface::~DaemonInterface()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    bus.unregisterObject(OBJECT_PATH);
    bus.unregisterService(SERVICE_NAME);
}

bool DaemonInterface::registerWithDBus()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.registerService(SERVICE_NAME)) {
        qCWarning(lcDaemon, "Couldn't register %s D-Bus service: %s",
                  qPrintable(SERVICE_NAME),
                  qPrintable(bus.lastError().message()));
        return false;
    }

    if (!bus.registerObject(OBJECT_PATH, this)) {
        qCWarning(lcDaemon, "Couldn't register %s D-Bus object: %s",
                  qPrintable(OBJECT_PATH),
                  qPrintable(bus.lastError().message()));
        return false;
    }

    return true;
}

void DaemonInterface::LoadModule(const QString &name)
{
    Daemon::instance()->loadModule(name);
}

void DaemonInterface::UnloadModule(const QString &name)
{
    Daemon::instance()->unloadModule(name);
}
