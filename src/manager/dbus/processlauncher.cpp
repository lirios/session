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
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

#include <LiriXdg/AutoStart>
#include <LiriXdg/DesktopFile>

#include "dbus/processlauncher.h"
#include "session.h"

ProcessLauncher::ProcessLauncher(QObject *parent)
    : QObject(parent)
    , m_session(qobject_cast<Session *>(parent))
{
}

ProcessLauncher::~ProcessLauncher()
{
    // Unregister D-Bus object
    auto bus = QDBusConnection::sessionBus();
    bus.unregisterObject(objectPath);
    bus.unregisterService(serviceName);
}

bool ProcessLauncher::registerWithDBus()
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
    if (!bus.registerObject(objectPath, serviceName,
                            this, QDBusConnection::ExportScriptableContents)) {
        qCWarning(lcSession,
                  "Failed to register \"%s\" D-Bus interface: %s",
                  qPrintable(serviceName),
                  qPrintable(bus.lastError().message()));
        return false;
    }

    return true;
}

bool ProcessLauncher::LaunchApplication(const QString &appId)
{
    if (appId.isEmpty())
        return false;

    const QString fileName = QStandardPaths::locate(
                QStandardPaths::ApplicationsLocation,
                appId + QStringLiteral(".desktop"));
    if (fileName.isEmpty()) {
        qCWarning(lcSession) << "Cannot find" << appId << "desktop file";
        return false;
    }

    auto *desktop = Liri::DesktopFileCache::getFile(fileName);
    if (!desktop) {
        qCWarning(lcSession) << "No desktop file found for" << appId;
        return false;
    }

    if (m_session->isSystemdEnabled() && !desktop->isDBusActivatable()) {
        // Run with systemd-run
        QStringList args = QStringList()
                << QStringLiteral("--user")
                << QStringLiteral("--scope")
                << QStringLiteral("--description=Application %1").arg(appId)
                << QStringLiteral("--property=Requisite=liri-shell.target")
                << QStringLiteral("--property=After=liri-shell.target")
                << QStringLiteral("--property=BindsTo=liri-session.target")
                << desktop->expandExecString().join(QLatin1Char(' '));

        QProcess *process = new QProcess(this);
        process->setProgram(QStringLiteral("systemd-run"));
        process->setArguments(args);
        connect(process, &QProcess::readyReadStandardOutput,
                this, &ProcessLauncher::handleReadyReadStandardOutput);
        connect(process, &QProcess::readyReadStandardError,
                this, &ProcessLauncher::handleReadyReadStandardError);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ProcessLauncher::handleProcessFinished);
        process->start();
        return process->waitForStarted();
    } else {
        return desktop->startDetached();
    }
}

bool ProcessLauncher::LaunchDesktopFile(const QString &path, const QStringList &urls)
{
    if (path.isEmpty())
        return false;

    auto *desktop = Liri::DesktopFileCache::getFile(path);
    if (!desktop) {
        qCWarning(lcSession) << "Failed to open desktop file" << path;
        return false;
    }

    if (m_session->isSystemdEnabled() && !desktop->isDBusActivatable()) {
        // Run with systemd-run
        const QString appId = id(path);
        QStringList args = QStringList()
                << QStringLiteral("--user")
                << QStringLiteral("--scope")
                << QStringLiteral("--description=Application %1").arg(appId)
                << QStringLiteral("--property=SourcePath=%1").arg(path)
                << QStringLiteral("--property=Requisite=liri-shell.target")
                << QStringLiteral("--property=After=liri-shell.target")
                << QStringLiteral("--property=BindsTo=liri-session.target")
                << desktop->expandExecString(urls).join(QLatin1Char(' '));

        QProcess *process = new QProcess(this);
        process->setProgram(QStringLiteral("systemd-run"));
        process->setArguments(args);
        connect(process, &QProcess::readyReadStandardOutput,
                this, &ProcessLauncher::handleReadyReadStandardOutput);
        connect(process, &QProcess::readyReadStandardError,
                this, &ProcessLauncher::handleReadyReadStandardError);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ProcessLauncher::handleProcessFinished);
        process->start();
        return process->waitForStarted();
    } else {
        return desktop->startDetached(urls);
    }
}

bool ProcessLauncher::LaunchCommand(const QString &command)
{
    if (command.isEmpty())
        return false;

    if (m_session->isSystemdEnabled()) {
        // Run with systemd-run
        QStringList args = QStringList()
                << QStringLiteral("--user")
                << QStringLiteral("--scope")
                << QStringLiteral("--description=Run command: %1").arg(command)
                << QStringLiteral("--property=Requisite=liri-shell.target")
                << QStringLiteral("--property=After=liri-shell.target")
                << QStringLiteral("--property=BindsTo=liri-session.target")
                << command;

        QProcess *process = new QProcess(this);
        process->setProgram(QStringLiteral("systemd-run"));
        process->setArguments(args);
        connect(process, &QProcess::readyReadStandardOutput,
                this, &ProcessLauncher::handleReadyReadStandardOutput);
        connect(process, &QProcess::readyReadStandardError,
                this, &ProcessLauncher::handleReadyReadStandardError);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ProcessLauncher::handleProcessFinished);
        process->start();
        return process->waitForStarted();
    } else {
        QProcess *process = new QProcess(this);
        return process->startDetached(command);
    }
}

QString ProcessLauncher::id(const QString &fileName) const
{
    const QFileInfo info(fileName);

    QRegularExpression suffixRx(QStringLiteral(".desktop$"));
    QLatin1Char slashChar('/');
    QLatin1Char dashChar('-');

    QString id = info.canonicalFilePath();
    id.replace(info.canonicalPath(), QString());
    if (id.startsWith(slashChar))
        id.remove(0, 1);
    id.replace(slashChar, dashChar);
    id.replace(suffixRx, QString());

    return id;
}

void ProcessLauncher::handleReadyReadStandardOutput()
{
    auto *process = qobject_cast<QProcess *>(sender());
    if (process)
        qCInfo(lcSession) << process->readAllStandardOutput().constData();
}

void ProcessLauncher::handleReadyReadStandardError()
{
    auto *process = qobject_cast<QProcess *>(sender());
    if (process)
        qCWarning(lcSession) << process->readAllStandardError().constData();
}

void ProcessLauncher::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    auto *process = qobject_cast<QProcess *>(sender());
    if (process)
        process->deleteLater();
}
