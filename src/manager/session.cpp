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

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>

#include <libsigwatch/sigwatch.h>

#include "diagnostics.h"
#include "gitsha1.h"
#include "pluginregistry.h"
#include "session.h"
#include "sessionmanager.h"
#include "utils.h"

Q_LOGGING_CATEGORY(lcSession, "liri.session", QtInfoMsg)

Q_IMPORT_PLUGIN(AutostartPlugin)
Q_IMPORT_PLUGIN(ServicesPlugin)
Q_IMPORT_PLUGIN(ShellPlugin)

Session::Session(QObject *parent)
    : QObject(parent)
    , m_pluginRegistry(new PluginRegistry(this))
    , m_manager(new SessionManager(this))
{
    // Unix signals watcher
    UnixSignalWatcher *sigwatch = new UnixSignalWatcher(this);
    sigwatch->watchForSignal(SIGINT);
    sigwatch->watchForSignal(SIGTERM);

    // Quit when the process is killed
    connect(sigwatch, &UnixSignalWatcher::unixSignal,
            this, &Session::shutdown);
}

Session::~Session()
{
    qDeleteAll(m_loadedModules);
}

bool Session::requireDBusSession()
{
    // Don't continue if we are already in a D-Bus session
    if (qEnvironmentVariableIsSet("DBUS_SESSION_BUS_ADDRESS"))
        return false;

    // Avoid infinite recursion if dbus-run-session fails to set DBUS_SESSION_BUS_ADDRESS
    if (getParentProcessName().contains(QStringLiteral("dbus-run-session")))
        return false;

    // Restart with dbus-run-session
    QCoreApplication::quit();
    QStringList args = QStringList() << QStringLiteral("--");
    args += QCoreApplication::arguments();
    QProcess::startDetached(QStringLiteral("dbus-run-session"),
                            args);
    return true;
}

void Session::disableModule(const QString &name)
{
    m_disabledModules.append(name);
}

void Session::setModuleArguments(const QString &name, const QStringList &args)
{
    m_moduleArgs[name] = args;
}

QProcessEnvironment Session::sessionEnvironment() const
{
    QProcessEnvironment env;

    QMap<QString, QString>::iterator it;
    QMap<QString, QString> sessionEnv = m_env;
    for (it = sessionEnv.begin(); it != sessionEnv.end(); ++it)
        env.insert(it.key(), it.value());

    return env;
}

bool Session::initialize()
{
    // Print version information
    qInfo("== Liri Session ==\n"
          "** https://liri.io\n"
          "** Bug reports to: https://github.com/lirios/session/issues\n"
          "** Build: %s-%s",
          LIRI_SESSION_VERSION, GIT_REV);

    // Print OS information
    qInfo("%s", qPrintable(Diagnostics::systemInformation().trimmed()));

    // Add environment variables
    extendSessionEnvironment();

    // Register D-Bus objects
    if (!m_manager->registerWithDBus())
        return false;

    // Discover plugins
    m_pluginRegistry->discover();

    // Add modules to the list
    PluginsMap plugins = m_pluginRegistry->plugins();
    PluginsMap::iterator it;
    for (it = plugins.begin(); it != plugins.end(); ++it) {
        auto name = it.key();
        auto instance = it.value();
        auto module = dynamic_cast<Liri::SessionModule *>(instance);

        if (!module) {
            qCWarning(lcSession, "Plugin \"%s\" is not a session module",
                      qPrintable(name));
            continue;
        }

        m_modules[module->startupPhase()].append(module);
    }

    return true;
}

bool Session::start()
{
    // Run all modules of each startup phase
    ModulesMap::iterator it;
    for (it = m_modules.begin(); it != m_modules.end(); ++it) {
        ModulesList list = it.value();
        for (int i = 0; i < list.count(); i++) {
            auto module = list.at(i);
            auto name = m_pluginRegistry->getNameForInstance(module);
            if (!module) {
                qCWarning(lcSession, "Ignoring invalid session module \"%s\"",
                          qPrintable(name));
                continue;
            }

            // Skip disabled modules
            if (m_disabledModules.contains(name))
                continue;

            // Let the module set environment variables directly
            connect(module, &Liri::SessionModule::environmentChangeRequested,
                    this, &Session::setEnvironment);

            qCInfo(lcSession, "==> Starting session module \"%s\"",
                   qPrintable(name));

            // Pass environment variables to the module
            passEnvironment(module);

            // Start
            if (module->start(m_moduleArgs[name])) {
                m_loadedModules.append(module);
                qCInfo(lcSession, "Session module \"%s\" started",
                       qPrintable(name));
            } else {
                qCWarning(lcSession, "Failed to start session module \"%s\"",
                          qPrintable(name));
                shutdown();
                return false;
            }

            // Propagate environment variables to D-Bus activate services
            QProcess::startDetached(QStringLiteral("dbus-update-activation-environment --systemd --all"));
        }
    }

    return true;
}

void Session::setEnvironment(const QString &key, const QString &value)
{
    qCDebug(lcSession, "Setting environment variable %s=\"%s\"",
            qPrintable(key), qPrintable(value));
    m_env[key] = value;
}

void Session::unsetEnvironment(const QString &key)
{
    qCDebug(lcSession, "Unsetting environment variable %s",
            qPrintable(key));
    m_env.remove(key);
}

void Session::shutdown()
{
    qCInfo(lcSession, "Closing session now");

    // Stop modules
    std::reverse(m_loadedModules.begin(), m_loadedModules.end());
    for (auto module : qAsConst(m_loadedModules)) {
        auto instance = dynamic_cast<QObject *>(module);
        const auto name = m_pluginRegistry->getNameForInstance(instance);

        qCInfo(lcSession, "==> Stopping session module \"%s\"",
               qPrintable(name));

        if (!module->stop())
            qCWarning(lcSession, "Failed to stop session module \"%s\"",
                      qPrintable(name));
    }

    qCInfo(lcSession, "Quitting");
    QCoreApplication::quit();
}

void Session::extendSessionEnvironment()
{
    // We need to pass all our environment variables to children
    // otherwise not everything will work
    auto systemEnv = QProcessEnvironment::systemEnvironment();
    const auto keys = systemEnv.keys();
    for (const auto &key : keys)
        m_env[key] = systemEnv.value(key);
}

void Session::passEnvironment(Liri::SessionModule *module)
{
    qCDebug(lcSession, "Passing session environment variables:");

    QMap<QString, QString>::iterator it;
    for (it = m_env.begin(); it != m_env.end(); ++it) {
        qCDebug(lcSession, "  %s=\"%s\"", qPrintable(it.key()), qPrintable(it.value()));
        module->setEnvironment(it.key(), it.value());
    }
}
