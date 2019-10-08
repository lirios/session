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
#include <QDBusMetaType>
#include <QDBusReply>
#include <QProcess>

#include <libsigwatch/sigwatch.h>

#include "dbus/screensaver.h"
#include "dbus/sessionmanager.h"
#include "diagnostics.h"
#include "gitsha1.h"
#include "pluginregistry.h"
#include "session.h"
#include "systemdmanager.h"
#include "utils.h"

Q_LOGGING_CATEGORY(lcSession, "liri.session", QtInfoMsg)

Q_IMPORT_PLUGIN(AutostartPlugin)
Q_IMPORT_PLUGIN(ServicesPlugin)
Q_IMPORT_PLUGIN(ShellPlugin)

// D-Bus type to update activation environment
typedef QMap<QString,QString> EnvMap;
Q_DECLARE_METATYPE(EnvMap)

Session::Session(QObject *parent)
    : QObject(parent)
    , m_screenSaver(new ScreenSaver(this))
    , m_manager(new SessionManager(this))
    , m_pluginRegistry(new PluginRegistry(this))
{
    // Register D-Bus types
    qDBusRegisterMetaType<EnvMap>();

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

bool Session::isSystemdEnabled() const
{
    return m_systemdEnabled;
}

void Session::setSystemdEnabled(bool value)
{
    if (m_systemdEnabled && !value) {
        qCWarning(lcSession, "Systemd support cannot be disabled once the program has started");
        return;
    }

    m_systemdEnabled = value;
    m_systemd = new SystemdManager(this);
}

bool Session::requireDBusSession()
{
    // Don't continue if we are already in a D-Bus session
    if (qEnvironmentVariableIsSet("DBUS_SESSION_BUS_ADDRESS"))
        return false;

    if (m_systemdEnabled) {
        if (!m_systemd->isAvailable()) {
            qCCritical(lcSession, "Systemd D-Bus service is not available");
            return false;
        }
    } else {
        // Avoid infinite recursion if dbus-run-session fails to set DBUS_SESSION_BUS_ADDRESS
        if (getParentProcessName().contains(QStringLiteral("dbus-run-session")))
            return false;

        // Restart with dbus-run-session
        QCoreApplication::quit();
        QStringList args = QStringList() << QStringLiteral("--");
        args += QCoreApplication::arguments();
        QProcess::startDetached(QStringLiteral("dbus-run-session"),
                                args);
    }

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

    // Register D-Bus objects
    m_screenSaver->registerWithDBus();
    if (!m_manager->registerWithDBus())
        return false;

    if (!m_systemdEnabled) {
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
    }

    return true;
}

bool Session::start()
{
    // Update D-Bus activation environment
    uploadEnvironment();

    bool status = false;

    if (m_systemdEnabled) {
        if (m_systemd->loadUnit(QStringLiteral("liri-session.target")))
            status = m_systemd->startUnit(QStringLiteral("liri-session.target"), QStringLiteral("replace"));
    } else {
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
                connect(module, &Liri::SessionModule::shutdownRequested,
                        this, &Session::shutdown);

                qCInfo(lcSession, "==> Starting session module \"%s\"",
                       qPrintable(name));

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
            }
        }

        status = true;
    }

    return status;
}

void Session::setEnvironment(const QString &key, const QString &value)
{
    // Set environment variable
    qCDebug(lcSession, "Setting environment variable %s=\"%s\"",
            qPrintable(key), qPrintable(value));
    qputenv(qPrintable(key), value.toLocal8Bit());

    // Propagate environment variable to the launcher
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("SetEnvironment"));
    msg.setArguments(QVariantList() << key << value);
    QDBusConnection::sessionBus().send(msg);

    // Propagate environment variables to D-Bus activate services
    uploadEnvironment();
}

void Session::unsetEnvironment(const QString &key)
{
    // Unset environment variable
    qCDebug(lcSession, "Unsetting environment variable %s",
            qPrintable(key));
    qunsetenv(qPrintable(key));

    // Propagate environment variable to the launcher
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("UnsetEnvironment"));
    msg.setArguments(QVariantList() << key);
    QDBusConnection::sessionBus().send(msg);

    // Propagate environment variables to D-Bus activate services
    uploadEnvironment();
}

void Session::shutdown()
{
    if (!m_systemdEnabled) {
        qCInfo(lcSession, "Closing session...");

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

        qCInfo(lcSession, "Quit");
    }

    QCoreApplication::quit();
}

void Session::uploadEnvironment()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QProcessEnvironment sysEnv(env);

    // Copy environment variables and remove login-session specific that we
    // don't want to set for every session
    sysEnv.remove(QStringLiteral("XDG_SEAT"));
    sysEnv.remove(QStringLiteral("XDG_SESSION_CLASS"));
    sysEnv.remove(QStringLiteral("XDG_SESSION_DESKTOP"));
    sysEnv.remove(QStringLiteral("XDG_SESSION_ID"));
    sysEnv.remove(QStringLiteral("XDG_SESSION_TYPE"));
    sysEnv.remove(QStringLiteral("XDG_VTNR"));

    // Avoid shell code in environment variables
    const auto keys = sysEnv.keys();
    for (const auto &key : keys) {
        if (key.startsWith(QStringLiteral("BASH_FUNC")))
            sysEnv.remove(key);
    }

    // Synchronously update activation environment
    {
        EnvMap envMap;
        const auto keys = sysEnv.keys();
        for (const auto &key : keys)
            envMap.insert(key, sysEnv.value(key));

        auto msg = QDBusMessage::createMethodCall(
                    QStringLiteral("org.freedesktop.DBus"),
                    QStringLiteral("/org/freedesktop/DBus"),
                    QStringLiteral("org.freedesktop.DBus"),
                    QStringLiteral("UpdateActivationEnvironment"));
        msg.setAutoStartService(false);
        msg.setArguments(QVariantList({QVariant::fromValue(envMap)}));
        QDBusReply<void> reply = QDBusConnection::sessionBus().call(msg);
        if (!reply.isValid())
            qCWarning(lcSession, "Failed to update activation environment: %s",
                      qPrintable(reply.error().message()));
    }

    // Synchronously update systemd environment
    if (m_systemdEnabled) {
        auto msg = QDBusMessage::createMethodCall(
                    QStringLiteral("org.freedesktop.systemd1"),
                    QStringLiteral("/org/freedesktop/systemd1"),
                    QStringLiteral("org.freedesktop.systemd1.Manager"),
                    QStringLiteral("SetEnvironment"));
        msg.setAutoStartService(false);
        msg.setArguments(QVariantList() << sysEnv.toStringList());
        QDBusReply<void> reply = QDBusConnection::sessionBus().call(msg);
        if (!reply.isValid())
            qCWarning(lcSession, "Failed to update systemd environment: %s",
                      qPrintable(reply.error().message()));
    }
}
