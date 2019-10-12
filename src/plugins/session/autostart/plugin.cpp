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
#include <QDBusMessage>
#include <QDBusPendingCall>

#include <LiriXdg/AutoStart>
#include <LiriXdg/DesktopFile>

#include "plugin.h"

AutostartPlugin::AutostartPlugin(QObject *parent)
    : Liri::SessionModule(parent)
{
}

Liri::SessionModule::StartupPhase AutostartPlugin::startupPhase() const
{
    return Applications;
}

bool AutostartPlugin::start(const QStringList &args)
{
    Q_UNUSED(args)

    const auto desktopFileList = Liri::AutoStart::desktopFileList();
    for (const Liri::DesktopFile &entry : desktopFileList) {
        // Ignore hidden entries
        if (!entry.isVisible())
            continue;

        // Ignore entries that are explicitely not meant for Liri
        if (!entry.isSuitable(QStringLiteral("X-Liri")))
            continue;

        // If it's neither suitable for GNOME nor KDE then it's probably not meant
        // for us too, some utilities like those from XFCE have an explicit list
        // of desktop that are not supported instead of show them on XFCE
        //if (!entry.isSuitable(true, QLatin1String("GNOME")) && !entry.isSuitable(true, QLatin1String("KDE")))
        //continue;

        // Ignore those entries hidden under systemd
        if (isSystemdEnabled()) {
            bool hidden = entry.value(QStringLiteral("X-GNOME-HiddenUnderSystemd")).toString().trimmed().compare(QStringLiteral("true"), Qt::CaseInsensitive);
            if (hidden)
                continue;
        }

        qCDebug(lcSession) << "Autostart entry:" << entry.name() << "from" << entry.fileName();
        m_desktopFiles.append(entry.fileName());
        launchDesktopFile(entry.fileName());
    }

    return true;
}

bool AutostartPlugin::stop()
{
    std::reverse(m_desktopFiles.begin(), m_desktopFiles.end());

    for (const auto &fileName : qAsConst(m_desktopFiles)) {
        qCDebug(lcSession) << "Terminate autostart entry from" << fileName;
        terminateDesktopFile(fileName);
    }

    m_desktopFiles.clear();

    return true;
}

void AutostartPlugin::launchDesktopFile(const QString &fileName)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("LaunchDesktopFile"));
    QVariantList args;
    args.append(fileName);
    msg.setArguments(args);
    QDBusConnection::sessionBus().asyncCall(msg);
}

void AutostartPlugin::terminateDesktopFile(const QString &fileName)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("TerminateDesktopFile"));
    QVariantList args;
    args.append(fileName);
    msg.setArguments(args);
    QDBusConnection::sessionBus().asyncCall(msg);
}
