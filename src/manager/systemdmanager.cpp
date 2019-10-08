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
#include <QDBusConnectionInterface>
#include <QDBusReply>

#include "session.h"
#include "systemdmanager.h"

SystemdManager::SystemdManager(QObject *parent)
    : QObject(parent)
{
    // Search for systemd
    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().interface()->call(QStringLiteral("ListNames"));
    m_available = reply.value().contains(QLatin1String("org.freedesktop.systemd1"));
}

bool SystemdManager::isAvailable() const
{
    return m_available;
}

bool SystemdManager::loadUnit(const QString &name)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.systemd1"),
                QStringLiteral("/org/freedesktop/systemd1"),
                QStringLiteral("org.freedesktop.systemd1.Manager"),
                QStringLiteral("LoadUnit"));
    msg.setAutoStartService(false);
    msg.setArguments(QVariantList() << name);
    QDBusReply<QDBusObjectPath> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCWarning(lcSession, "Unable to load unit \"%s\": %s",
                  qPrintable(name), qPrintable(reply.error().message()));
        return false;
    }

    return true;
}

bool SystemdManager::startUnit(const QString &name, const QString &mode)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.systemd1"),
                QStringLiteral("/org/freedesktop/systemd1"),
                QStringLiteral("org.freedesktop.systemd1.Manager"),
                QStringLiteral("StartUnit"));
    msg.setAutoStartService(false);
    msg.setArguments(QVariantList() << name << mode);
    QDBusReply<QDBusObjectPath> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCWarning(lcSession, "Unable to start unit \"%s\": %s",
                  qPrintable(name), qPrintable(reply.error().message()));
        return false;
    }

    return true;
}

bool SystemdManager::stopUnit(const QString &name, const QString &mode)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.systemd1"),
                QStringLiteral("/org/freedesktop/systemd1"),
                QStringLiteral("org.freedesktop.systemd1.Manager"),
                QStringLiteral("StopUnit"));
    msg.setAutoStartService(false);
    msg.setArguments(QVariantList() << name << mode);
    QDBusReply<QDBusObjectPath> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCWarning(lcSession, "Unable to stop unit \"%s\": %s",
                  qPrintable(name), qPrintable(reply.error().message()));
        return false;
    }

    return true;
}

bool SystemdManager::setEnvironment(const QProcessEnvironment &sysEnv)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.systemd1"),
                QStringLiteral("/org/freedesktop/systemd1"),
                QStringLiteral("org.freedesktop.systemd1.Manager"),
                QStringLiteral("SetEnvironment"));
    msg.setAutoStartService(false);
    msg.setArguments(QVariantList() << sysEnv.toStringList());
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCWarning(lcSession, "Failed to update systemd environment: %s",
                  qPrintable(reply.error().message()));
        return false;
    }

    return true;
}

bool SystemdManager::unsetEnvironment(const QString &key)
{
    return unsetEnvironment(QStringList() << key);
}

bool SystemdManager::unsetEnvironment(const QStringList &keys)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.systemd1"),
                QStringLiteral("/org/freedesktop/systemd1"),
                QStringLiteral("org.freedesktop.systemd1.Manager"),
                QStringLiteral("UnsetEnvironment"));
    msg.setAutoStartService(false);
    msg.setArguments(QVariantList() << keys);
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCWarning(lcSession, "Failed to unset environment variables from systemd: %s",
                  qPrintable(reply.error().message()));
        return false;
    }

    return true;
}
