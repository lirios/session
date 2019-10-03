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
#include <QDBusServiceWatcher>
#include <QEventLoop>
#include <QFile>
#include <QTimer>

#include "plugin.h"

const QString shellServiceName = QStringLiteral("io.liri.Shell");

ShellPlugin::ShellPlugin(QObject *parent)
    : Liri::SessionModule(parent)
{
    m_loop = new QEventLoop(this);

    m_serviceWatcher =
            new QDBusServiceWatcher(shellServiceName, QDBusConnection::sessionBus(),
                                    QDBusServiceWatcher::WatchForRegistration |
                                    QDBusServiceWatcher::WatchForUnregistration,
                                    this);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, &ShellPlugin::handleServiceRegistered);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &ShellPlugin::handleServiceUnregistered);

    m_serverProcess = new QProcess(this);
    m_serverProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    m_serverProcess->setProgram(QString::asprintf("%s/liri-shell", LIBEXECDIR));

    connect(m_serverProcess, &QProcess::readyReadStandardOutput,
            this, &ShellPlugin::handleStandardOutput);
    connect(m_serverProcess, &QProcess::readyReadStandardError,
            this, &ShellPlugin::handleStandardError);
    connect(m_serverProcess, &QProcess::errorOccurred,
            this, &ShellPlugin::processCrashed);
    connect(m_serverProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ShellPlugin::processFinished);
}

Liri::SessionModule::StartupPhase ShellPlugin::startupPhase() const
{
    return WindowManager;
}

bool ShellPlugin::start(const QStringList &args)
{
    // Save the effort of running it, if the executable doesn't exist
    if (!QFile::exists(m_serverProcess->program())) {
        qCWarning(lcSession, "Couldn't find the \"%s\" executable, "
                             "please check your installation",
                  qPrintable(m_serverProcess->program()));
        return false;
    }

    // Set arguments
    m_serverProcess->setArguments(args);

    // Set environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove(QStringLiteral("QT_QPA_PLATFORM"));
    env.remove(QStringLiteral("QT_WAYLAND_SHELL_INTEGRATION"));
    m_serverProcess->setProcessEnvironment(env);

    // Run with retries
    int retries = 5;
    while (retries-- > 0) {
        qCWarning(lcSession, "Trying to run liri-shell...");
        m_serverProcess->start();
        if (m_serverProcess->waitForStarted()) {
            // The process has started, but we can't continue until the D-Bus
            // service is up, that is when the shell is really up and running
            QTimer::singleShot(30 * 1000, m_loop, &QEventLoop::quit);
            m_loop->exec();
            return m_serverProcess->state() == QProcess::Running;
        } else {
            if (retries == 0)
                qCWarning(lcSession, "Failed to start liri-shell, giving up!");
            else
                qCWarning(lcSession,
                          "Failed to start liri-shell, %d attempt(s) left",
                          retries);
        }
    }

    return false;
}

bool ShellPlugin::stop()
{
    m_stopping = true;

    if (m_helperProcess) {
        m_helperProcess->terminate();
        if (!m_helperProcess->waitForFinished())
            m_helperProcess->kill();
    }

    if (m_serverProcess) {
        m_serverProcess->terminate();
        if (!m_serverProcess->waitForFinished())
            m_serverProcess->kill();
    }

    m_stopping = false;

    return true;
}

void ShellPlugin::startShellHelper()
{
    m_helperProcess = new QProcess(this);
    m_helperProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    m_helperProcess->setProgram(QString::asprintf("%s/liri-shell-helper", LIBEXECDIR));

    connect(m_helperProcess, &QProcess::readyReadStandardOutput,
            this, &ShellPlugin::handleStandardOutput);
    connect(m_helperProcess, &QProcess::readyReadStandardError,
            this, &ShellPlugin::handleStandardError);

    m_helperProcess->start();
    if (!m_helperProcess->waitForStarted()) {
        // Can't continue without the helper
        qCWarning(lcSession, "Can't start liri-shell-helper");
        if (m_serverProcess) {
            m_serverProcess->terminate();
            if (!m_serverProcess->waitForFinished())
                m_serverProcess->kill();
        }
        Q_EMIT shutdownRequested();
    }
}

void ShellPlugin::handleServiceRegistered(const QString &serviceName)
{
    if (serviceName == shellServiceName) {
        // We don't need to list for this signal
        disconnect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,
                   this, &ShellPlugin::handleServiceRegistered);

        // The shell D-Bus service is registered, this means it's ready
        // and we can move on to the next session module
        m_loop->quit();

        // Now start the helper
        startShellHelper();
    }
}

void ShellPlugin::handleServiceUnregistered(const QString &serviceName)
{
    if (serviceName == shellServiceName) {
        // We don't need to list for this signal
        disconnect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
                   this, &ShellPlugin::handleServiceUnregistered);

        // Can't continue without the display server
        Q_EMIT shutdownRequested();
    }
}

void ShellPlugin::handleStandardOutput()
{
    auto *process = qobject_cast<QProcess *>(sender());
    if (process)
        qInfo() << process->readAllStandardOutput();
}

void ShellPlugin::handleStandardError()
{
    auto *process = qobject_cast<QProcess *>(sender());
    if (process)
        qCritical() << process->readAllStandardError();
}

void ShellPlugin::processCrashed(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        qCWarning(lcSession,
                  "Failed to start \"%s\": check if liri-shell is installed correctly",
                  qPrintable(m_serverProcess->program()));
        m_loop->quit();
        break;
    case QProcess::Crashed:
        qCWarning(lcSession,
                  "Program \"%s\" just crashed", qPrintable(m_serverProcess->program()));
        m_loop->quit();
        if (m_serverProcess->state() == QProcess::NotRunning && !m_stopping) {
            if (m_watchDogCounter-- > 0)
                m_serverProcess->start();
        }
        break;
    case QProcess::UnknownError:
        qCWarning(lcSession, "An unknown error occurred starting \"%s\"",
                  qPrintable(m_serverProcess->program()));
        break;
    default:
        break;
    }
}

void ShellPlugin::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0)
        qCWarning(lcSession,
                  "\"%s\" finished with exit code %d",
                  qPrintable(m_serverProcess->program()), exitCode);
}
