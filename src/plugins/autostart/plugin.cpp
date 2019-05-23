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

#include <LiriXdg/AutoStart>

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

    for (const Liri::DesktopFile &entry : Liri::AutoStart::desktopFileList()) {
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

        qCDebug(lcSession) << "Autostart:" << entry.name() << "from" << entry.fileName();
        launchEntry(entry);
    }

    return true;
}

bool AutostartPlugin::stop()
{
    std::reverse(m_processes.begin(), m_processes.end());

    for (auto process : qAsConst(m_processes)) {
        process->terminate();
        if (!process->waitForFinished())
            process->kill();
    }

    return true;
}

bool AutostartPlugin::launchEntry(const Liri::DesktopFile &entry)
{
    QStringList args = entry.expandExecString();
    QString command = args.takeAt(0);

    qCDebug(lcSession)
            << "Launching" << entry.expandExecString().join(QStringLiteral(" "))
            << "from" << entry.fileName();

    QProcessEnvironment env;
    QMap<QString, QString>::iterator it;
    QMap<QString, QString> sessionEnv = environment();
    for (it = sessionEnv.begin(); it != sessionEnv.end(); ++it)
        env.insert(it.key(), it.value());

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
            this, &AutostartPlugin::handleStandardOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &AutostartPlugin::handleStandardError);
    connect(process, &QProcess::started,
            this, &AutostartPlugin::processStarted);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AutostartPlugin::processFinished);
    m_processes.append(process);
    if (!process->startDetached()) {
        qCWarning(lcSession, "Failed to launch \"%s\" (%s)",
                  qPrintable(entry.fileName()),
                  qPrintable(entry.name()));
        return false;
    }

    return true;
}

void AutostartPlugin::handleStandardOutput()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    qInfo() << process->readAllStandardOutput();
}

void AutostartPlugin::handleStandardError()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    qCritical() << process->readAllStandardError();
}

void AutostartPlugin::processStarted()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    qCDebug(lcSession, "Launched \"%s\" with pid %lld",
            qPrintable(process->program()),
            process->pid());
}

void AutostartPlugin::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    if (m_processes.contains(process)) {
        qCDebug(lcSession, "Program \"%s\" (pid %lld) finished with exit code %d",
                qPrintable(process->program()), process->pid(), exitCode);
        m_processes.removeOne(process);
    }
}
