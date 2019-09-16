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

#include <libsigwatch/sigwatch.h>

#include "daemon.h"
#include "daemoninterface.h"
#include "pluginregistry.h"

Q_LOGGING_CATEGORY(lcDaemon, "liri.daemon", QtInfoMsg)

Q_GLOBAL_STATIC(Daemon, s_daemon)

Daemon::Daemon(QObject *parent)
    : QObject(parent)
    , m_pluginRegistry(new PluginRegistry(this))
{
    // Unix signals watcher
    UnixSignalWatcher *sigwatch = new UnixSignalWatcher(this);
    sigwatch->watchForSignal(SIGINT);
    sigwatch->watchForSignal(SIGTERM);

    // Quit when the process is killed
    connect(sigwatch, &UnixSignalWatcher::unixSignal,
            this, &Daemon::shutdown);
}

bool Daemon::isSystemdEnabled() const
{
    return m_systemdEnabled;
}

void Daemon::setSystemdEnabled(bool value)
{
    if (m_systemdEnabled && !value) {
        qCWarning(lcDaemon, "Systemd support cannot be disabled once the program has started");
        return;
    }

    m_systemdEnabled = value;
}

bool Daemon::isShuttingDown() const
{
    return !m_running;
}

void Daemon::disableModule(const QString &name)
{
    m_disabledModules.append(name);
}

bool Daemon::initialize()
{
    // Create D-Bus service
    if (!isSystemdEnabled())
        m_interface = new DaemonInterface(this);

    // Register D-Bus objects
    if (m_interface && !m_interface->registerWithDBus())
        return false;

    // Discover plugins
    m_pluginRegistry->discover();

    // Add modules to the list
    PluginsMap plugins = m_pluginRegistry->plugins();
    PluginsMap::iterator it;
    for (it = plugins.begin(); it != plugins.end(); ++it) {
        auto name = it.key();
        auto instance = it.value();
        auto module = dynamic_cast<Liri::DaemonModule *>(instance);

        if (!module) {
            qCWarning(lcDaemon, "Plugin \"%s\" is not a daemon module",
                      qPrintable(name));
            continue;
        }

        // Remove the module when it is deleted
        connect(module, &Liri::DaemonModule::moduleDeleted, this, [this, module] {
            m_loadedModules.removeOne(module);
            m_modules.removeOne(module);
            m_modulesOnDemand.removeOne(module);
        });

        auto metaData = m_pluginRegistry->getMetaData(name);
        if (metaData.value(QStringLiteral("X-Liri-DaemonModule-AutoLoad"), true).toBool())
            m_modules.append(module);
        else
            m_modulesOnDemand.append(module);
    }

    return true;
}

void Daemon::start()
{
    // This is needed only in normal mode
    if (isSystemdEnabled())
        return;

    // Start all modules
    for (auto *module : qAsConst(m_modules)) {
        auto name = m_pluginRegistry->getNameForInstance(module);
        if (!module) {
            qCWarning(lcDaemon, "Ignoring invalid session module \"%s\"",
                      qPrintable(name));
            continue;
        }

        // Skip disabled modules
        if (m_disabledModules.contains(name))
            continue;

        // Start
        qCInfo(lcDaemon, "==> Starting module \"%s\"", qPrintable(name));
        module->start();
        m_loadedModules.append(module);
        qCInfo(lcDaemon, "Module \"%s\" started", qPrintable(name));
    }
}

void Daemon::shutdown()
{
    m_running = false;

    qCInfo(lcDaemon, "Stopping...");

    // Stop modules
    std::reverse(m_loadedModules.begin(), m_loadedModules.end());
    for (auto *module : qAsConst(m_loadedModules)) {
        auto *instance = dynamic_cast<QObject *>(module);
        const auto name = m_pluginRegistry->getNameForInstance(instance);

        // Stop
        unloadModule(name);
    }

    qCInfo(lcDaemon, "Bye");
    QCoreApplication::quit();
}

void Daemon::loadModule(const QString &name)
{
    // Skip disabled modules
    if (m_disabledModules.contains(name))
        return;

    // Find module
    auto *instance = m_pluginRegistry->getInstance(name);
    auto *module = qobject_cast<Liri::DaemonModule *>(instance);

    // Load module
    if (!m_loadedModules.contains(module)) {
        if (!m_modules.contains(module)) {
            qCWarning(lcDaemon, "Cannot find module \"%s\"", qPrintable(name));
            return;
        }

        qCInfo(lcDaemon, "==> Starting module \"%s\"", qPrintable(name));

        // Claim a D-Bus service name so that systemd knows it started
        if (isSystemdEnabled()) {
            auto metaData = m_pluginRegistry->getMetaData(name);
            auto serviceName = metaData[QStringLiteral("X-Liri-DaemonModule-ServiceName")].toString();
            if (!QDBusConnection::sessionBus().registerService(serviceName)) {
                qCWarning(lcDaemon, "Failed to register D-Bus service %s: %s",
                          qPrintable(serviceName),
                          qPrintable(QDBusConnection::sessionBus().lastError().message()));
                return;
            }
        }

        module->start();
        m_loadedModules.append(module);

        qCInfo(lcDaemon, "Module \"%s\" started", qPrintable(name));
    }
}

void Daemon::unloadModule(const QString &name)
{
    if (isShuttingDown())
        return;

    // Find module
    auto *instance = m_pluginRegistry->getInstance(name);
    auto *module = qobject_cast<Liri::DaemonModule *>(instance);

    // Unload module
    if (m_loadedModules.contains(module)) {
        qCInfo(lcDaemon, "==> Stopping module \"%s\"", qPrintable(name));

        module->stop();
        m_loadedModules.removeOne(module);

        qCInfo(lcDaemon, "Module \"%s\" stopped", qPrintable(name));

        // With systemd our sole purpose is to load a single module, quit if we are done
        if (isSystemdEnabled())
            QCoreApplication::quit();
    }
}

Daemon *Daemon::instance()
{
    return s_daemon();
}
