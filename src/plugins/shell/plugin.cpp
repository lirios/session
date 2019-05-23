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
                                    QDBusServiceWatcher::WatchForRegistration,
                                    this);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, &ShellPlugin::handleServiceRegistered);

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->setProgram(QString::asprintf("%s/liri-shell", LIBEXECDIR));

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ShellPlugin::handleStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &ShellPlugin::handleStandardError);
    connect(m_process, &QProcess::errorOccurred,
            this, &ShellPlugin::processCrashed);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ShellPlugin::processFinished);
}

Liri::SessionModule::StartupPhase ShellPlugin::startupPhase() const
{
    return WindowManager;
}

bool ShellPlugin::start(const QStringList &args)
{
    // Save the effort of running it, if the executable doesn't exist
    if (!QFile::exists(m_process->program())) {
        qCWarning(lcSession, "Couldn't find the \"%s\" executable, "
                             "please check your installation",
                  qPrintable(m_process->program()));
        return false;
    }

    // Set arguments
    m_process->setArguments(args);

    // Set environment
    QProcessEnvironment env;
    QMap<QString, QString>::iterator it;
    QMap<QString, QString> sessionEnv = environment();
    for (it = sessionEnv.begin(); it != sessionEnv.end(); ++it)
        env.insert(it.key(), it.value());
    m_process->setProcessEnvironment(env);

    // Run with retries
    int retries = 5;
    while (retries-- > 0) {
        qCWarning(lcSession, "Trying to run liri-shell...");
        m_process->start();
        if (m_process->waitForStarted()) {
            // The process has started, but we can't continue until the D-Bus
            // service is up, that is when the shell is really up and running
            QTimer::singleShot(30 * 1000, m_loop, &QEventLoop::quit);
            m_loop->exec();
            return true;
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

    m_process->terminate();
    if (!m_process->waitForFinished())
        m_process->kill();

    m_stopping = false;

    return true;
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
    }
}

void ShellPlugin::handleStandardOutput()
{
    qInfo() << m_process->readAllStandardOutput();
}

void ShellPlugin::handleStandardError()
{
    qCritical() << m_process->readAllStandardError();
}

void ShellPlugin::processCrashed(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        qCWarning(lcSession,
                  "Failed to start \"%s\": check if liri-shell is installed correctly",
                  qPrintable(m_process->program()));
        break;
    case QProcess::Crashed:
        qCWarning(lcSession,
                  "Program \"%s\" just crashed", qPrintable(m_process->program()));
        if (m_process->state() == QProcess::NotRunning && !m_stopping) {
            if (m_watchDogCounter-- > 0)
                start();
        }
        break;
    case QProcess::UnknownError:
        qCWarning(lcSession, "An unknown error occurred starting \"%s\"",
                  qPrintable(m_process->program()));
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
                  qPrintable(m_process->program()), exitCode);
}
