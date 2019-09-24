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
#include <QFile>
#include <QProcess>
#include <QStandardPaths>

#include <LiriXdg/DesktopFile>

#include "processlauncher.h"

Q_LOGGING_CATEGORY(lcLauncher, "liri.launcher", QtInfoMsg)

const QString serviceName = QStringLiteral("io.liri.Launcher");
const QString objectPath = QStringLiteral("/io/liri/Launcher");
const QString interfaceName = QStringLiteral("io.liri.Launcher");

ProcessLauncher::ProcessLauncher(QObject *parent)
    : QObject(parent)
{
    // Set environment for the programs we will launch from here
    qputenv("XDG_SESSION_TYPE", "wayland");
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    qputenv("QT_QPA_PLATFORM", "wayland;xcb");
#else
    qputenv("QT_QPA_PLATFORM", "wayland");
#endif
    qputenv("QT_QPA_PLATFORMTHEME", "liri");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell-v6");
    qputenv("QT_QUICK_CONTROLS_1_STYLE", "Flat");
    qputenv("QT_QUICK_CONTROLS_STYLE", "material");
    qputenv("QT_WAYLAND_DECORATION", "material");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    qputenv("XCURSOR_THEME", "Paper");
    qunsetenv("QT_SCALE_FACTOR");
    qunsetenv("QT_SCREEN_SCALE_FACTORS");
}

ProcessLauncher::~ProcessLauncher()
{
    // Unregister D-Bus object
    auto bus = QDBusConnection::sessionBus();
    bus.unregisterObject(objectPath);
    bus.unregisterService(serviceName);
}

void ProcessLauncher::run()
{
    auto bus = QDBusConnection::sessionBus();

    // Register D-Bus service
    if (!bus.registerService(serviceName))
        qCWarning(lcLauncher,
                  "Failed to register D-Bus service \"%s\": %s",
                  qPrintable(serviceName),
                  qPrintable(bus.lastError().message()));

    // Register D-Bus object
    if (!bus.registerObject(objectPath, interfaceName,
                            this, QDBusConnection::ExportScriptableContents))
        qCWarning(lcLauncher,
                  "Failed to register \"%s\" D-Bus interface: %s",
                  qPrintable(interfaceName),
                  qPrintable(bus.lastError().message()));
}

void ProcessLauncher::SetEnvironment(const QString &key, const QString &value)
{
    qCDebug(lcLauncher, "Setting environment variable %s=\"%s\"",
            qPrintable(key), qPrintable(value));
    qputenv(qPrintable(key), value.toUtf8());
}

void ProcessLauncher::UnsetEnvironment(const QString &key)
{
    qCDebug(lcLauncher, "Unsetting environment variable %s",
            qPrintable(key));
    qunsetenv(qPrintable(key));
}

bool ProcessLauncher::LaunchApplication(const QString &appId)
{
    if (appId.isEmpty())
        return false;

    const QString fileName = QStandardPaths::locate(
                QStandardPaths::ApplicationsLocation,
                appId + QStringLiteral(".desktop"));
    if (fileName.isEmpty()) {
        qCWarning(lcLauncher) << "Cannot find" << appId << "desktop file";
        return false;
    }

    Liri::DesktopFile *desktop = Liri::DesktopFileCache::getFile(fileName);
    if (!desktop) {
        qCWarning(lcLauncher) << "No desktop file found for" << appId;
        return false;
    }

    return desktop->startDetached();
}

bool ProcessLauncher::LaunchDesktopFile(const QString &path, const QStringList &urls)
{
    if (path.isEmpty())
        return false;

    Liri::DesktopFile *desktop = nullptr;

    if (QFile::exists(path))
        desktop = new Liri::DesktopFile(path);
    else
        desktop = Liri::DesktopFileCache::getFile(path);

    if (!desktop) {
        qCWarning(lcLauncher) << "Failed to open desktop file" << path;
        return false;
    }

    return desktop->startDetached(urls);
}

bool ProcessLauncher::LaunchCommand(const QString &command)
{
    if (command.isEmpty())
        return false;

    QProcess *process = new QProcess(this);
    return process->startDetached(command);
}
