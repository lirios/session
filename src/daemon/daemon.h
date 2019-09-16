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

#ifndef DAEMON_H
#define DAEMON_H

#include <QLoggingCategory>
#include <QObject>
#include <QProcessEnvironment>
#include <QVector>

#include <LiriDaemon/DaemonModule>

Q_DECLARE_LOGGING_CATEGORY(lcDaemon)

class DaemonInterface;
class PluginRegistry;

typedef QVector<Liri::DaemonModule *> ModulesList;

class Daemon : public QObject
{
    Q_OBJECT
public:
    explicit Daemon(QObject *parent = nullptr);

    bool isSystemdEnabled() const;
    void setSystemdEnabled(bool value);

    bool isShuttingDown() const;

    void disableModule(const QString &name);

    bool initialize();
    void start();

public Q_SLOTS:
    void shutdown();

public:
    void loadModule(const QString &name);
    void unloadModule(const QString &name);

    static Daemon *instance();

private:
    bool m_systemdEnabled = true;
    bool m_running = true;
    QStringList m_disabledModules;
    PluginRegistry *m_pluginRegistry = nullptr;
    ModulesList m_modules;
    ModulesList m_modulesOnDemand;
    ModulesList m_loadedModules;
    DaemonInterface *m_interface = nullptr;
};

#endif // DAEMON_H
