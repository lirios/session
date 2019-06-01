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
#include <QStandardPaths>

#include "session.h"
#include "sessionmanager.h"
#include "sessionmanager_adaptor.h"

const QString serviceName = QStringLiteral("io.liri.SessionManager");
const QString objectPath = QStringLiteral("/io/liri/SessionManager");

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_session(qobject_cast<Session *>(parent))
{
    new SessionManagerAdaptor(this);
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

void SessionManager::closeApplications()
{
    qCInfo(lcSession, "Terminate applications");

    // Terminate all process launched by us
    ApplicationMapIterator i(m_apps);
    while (i.hasNext()) {
        i.next();

        QString fileName = i.key();
        QProcess *process = i.value();

        i.remove();

        qCDebug(lcSession) << "Terminating application from" << fileName << "with pid"
                           << process->pid();

        process->terminate();
        if (!process->waitForFinished()) {
            process->kill();
            process->waitForFinished();
        }
        process->deleteLater();
    }
}

bool SessionManager::LaunchApplication(const QString &appId)
{
    if (appId.isEmpty())
        return false;

    const QString fileName = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                                    appId + QStringLiteral(".desktop"));
    if (fileName.isEmpty()) {
        qCWarning(lcSession) << "Cannot find" << appId << "desktop entry";
        return false;
    }

    Liri::DesktopFile *entry = Liri::DesktopFileCache::getFile(fileName);
    if (!entry) {
        qCWarning(lcSession) << "No desktop entry found for" << appId;
        return false;
    }

    return launchEntry(entry);
}

bool SessionManager::LaunchDesktopFile(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    Liri::DesktopFile *entry = Liri::DesktopFileCache::getFile(fileName);
    if (!entry) {
        qCWarning(lcSession()) << "Failed to open desktop file" << fileName;
        return false;
    }

    return launchEntry(entry);
}

bool SessionManager::LaunchCommand(const QString &command)
{
    if (!m_session)
        return false;
    if (command.isEmpty())
        return false;

    qCInfo(lcSession) << "Launching command" << command;

    QProcessEnvironment env = m_session->sessionEnvironment();

    // Applications should always see this is a Wayland session,
    // however we don't want to set this for liri-shell otherwise it
    // would interfere with its autodetection, so we put it here
    env.insert(QStringLiteral("XDG_SESSION_TYPE"), QStringLiteral("wayland"));

    QProcess *process = new QProcess(this);
    process->setProcessEnvironment(env);
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(process, &QProcess::readyReadStandardOutput,
            this, &SessionManager::handleStandardOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &SessionManager::handleStandardError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SessionManager::processFinished);
    process->start(command);
    if (!process->waitForStarted()) {
        qCWarning(lcSession) << "Failed to launch command" << command;
        return false;
    }

    qCInfo(lcSession) << "Launched command" << command << "with pid" << process->pid();

    return true;
}

void SessionManager::SetEnvironment(const QString &key, const QString &value)
{
    if (m_session)
        m_session->setEnvironment(key, value);
}

void SessionManager::Logout()
{
    if (m_session)
        m_session->shutdown();
}

bool SessionManager::launchEntry(Liri::DesktopFile *entry)
{
    if (!m_session)
        return false;

    if (entry->isDBusActivatable()) {
        const auto appId = Liri::DesktopFile::id(entry->fileName().replace(QStringLiteral(".desktop"), QString()));
        const auto dbusPath = QString(appId).replace(QLatin1Char('.'), QLatin1Char('/')).prepend(QLatin1Char('/'));

        qCDebug(lcSession)
                << "Activating" << appId << "from"
                << entry->fileName();

        auto msg = QDBusMessage::createMethodCall(appId, dbusPath,
                                                  QStringLiteral("org.freedesktop.Application"),
                                                  QStringLiteral("Activate"));
        QVariantMap platformData;
        QVariantList args;
        args.append(platformData);
        auto reply = QDBusConnection::sessionBus().call(msg, QDBus::BlockWithGui);

        return reply.type() != QDBusMessage::ErrorMessage;
    }

    QStringList args = entry->expandExecString();
    QString command = args.takeAt(0);

    qCDebug(lcSession)
            << "Launching"
            << entry->expandExecString().join(QLatin1Char(' ')) << "from"
            << entry->fileName();

    QProcessEnvironment env = m_session->sessionEnvironment();

    // Applications should always see this is a Wayland session,
    // however we don't want to set this for liri-shell otherwise it
    // would interfere with its autodetection, so we put it here
    env.insert(QStringLiteral("XDG_SESSION_TYPE"), QStringLiteral("wayland"));

    QProcess *process = new QProcess(this);
    process->setProgram(command);
    process->setArguments(args);
    process->setProcessEnvironment(env);
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(process, &QProcess::readyReadStandardOutput,
            this, &SessionManager::handleStandardOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &SessionManager::handleStandardError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SessionManager::processFinished);
    m_apps[entry->fileName()] = process;
    process->start();
    if (!process->waitForStarted()) {
        qCWarning(lcSession, "Failed to launch \"%s\" (%s)",
                  qPrintable(entry->fileName()),
                  qPrintable(entry->name()));
        return false;
    }

    qCDebug(lcSession, "Launched \"%s\" (%s) with pid %lld",
            qPrintable(entry->fileName()),
            qPrintable(entry->name()), process->pid());

    return true;
}

void SessionManager::handleStandardOutput()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    qInfo() << process->readAllStandardOutput();
}

void SessionManager::handleStandardError()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    qCritical() << process->readAllStandardError();
}

void SessionManager::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    QString fileName = m_apps.key(process);
    if (!fileName.isEmpty()) {
        qCDebug(lcSession) << "Application for" << fileName << "finished with exit code" << exitCode;
        m_apps.remove(fileName);
    }

    process->deleteLater();
}
