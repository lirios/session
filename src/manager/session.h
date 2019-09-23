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

#ifndef SESSION_H
#define SESSION_H

#include <QLoggingCategory>
#include <QObject>
#include <QProcessEnvironment>
#include <QVector>

#include <LiriSession/SessionModule>

Q_DECLARE_LOGGING_CATEGORY(lcSession)

class PluginRegistry;
class SessionManager;

typedef QVector<Liri::SessionModule *> ModulesList;
typedef QMap<Liri::SessionModule::StartupPhase, ModulesList> ModulesMap;

class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session(QObject *parent = nullptr);
    ~Session();

    bool requireDBusSession();

    void disableModule(const QString &name);
    void setModuleArguments(const QString &name,
                            const QStringList &args);

    bool initialize();
    bool start();

public Q_SLOTS:
    void setEnvironment(const QString &key, const QString &value);
    void unsetEnvironment(const QString &key);
    void shutdown();

private:
    QStringList m_disabledModules;
    QMap<QString, QStringList> m_moduleArgs;
    PluginRegistry *m_pluginRegistry = nullptr;
    SessionManager *m_manager = nullptr;
    ModulesMap m_modules;
    ModulesList m_loadedModules;

    void uploadEnvironment();
};

#endif // SESSION_H
