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

#include <QDir>
#include <QJsonObject>
#include <QPluginLoader>
#include <QStaticPlugin>
#include <QVector>

#include "daemon.h"
#include "pluginregistry.h"

PluginRegistry::PluginRegistry(QObject *parent)
    : QObject(parent)
{
}

PluginsMap PluginRegistry::plugins() const
{
    return m_plugins;
}

bool PluginRegistry::hasPlugin(const QString &name) const
{
    return m_plugins.contains(name);
}

QVariantMap PluginRegistry::getMetaData(const QString &name) const
{
    return m_pluginsMetaData[name];
}

QObject *PluginRegistry::getInstance(const QString &name) const
{
    if (!m_plugins.contains(name))
        return nullptr;
    return m_plugins[name];
}

QString PluginRegistry::getNameForInstance(QObject *instance) const
{
    return m_plugins.key(instance);
}

void PluginRegistry::discover()
{
    // Find static plugins first
    const auto staticPlugins = QPluginLoader::staticPlugins();
    for (QStaticPlugin staticPlugin : staticPlugins) {
        const auto json = staticPlugin.metaData().toVariantMap();
        addPlugin(staticPlugin.instance(), json);
    }

    // Find external plugins
    QDir pluginsDir(QString::asprintf("%s/liri/daemon", PLUGINSDIR));
    const auto entryList = pluginsDir.entryList(QDir::Files);
    for (const auto &fileName : entryList) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        addPlugin(loader.instance(), loader.metaData().toVariantMap());
    }
}

void PluginRegistry::addPlugin(QObject *instance, const QVariantMap &json)
{
    // Must have interface ID and metadata
    if (!json.contains(QStringLiteral("IID")) ||
            !json.contains(QStringLiteral("MetaData"))) {
        qCWarning(lcDaemon, "Ignoring invalid plugin");
        return;
    }

    // Check the interface ID, even though we only have this kind of plugins
    if (json[QStringLiteral("IID")] != QStringLiteral(LiriDaemonModule_iid))
        return;

    // Add to the list
    const auto metaData = json[QStringLiteral("MetaData")].toMap();
    const auto id = metaData.value(QStringLiteral("Id")).toString();
    const auto type = metaData.value(QStringLiteral("Type")).toString();
    if (type != QStringLiteral("DaemonModule")) {
        qCWarning(lcDaemon, "Plugin \"%s\" is of type %s instead of DaemonModule",
                  qPrintable(id), qPrintable(type));
        return;
    }
    m_plugins[id] = instance;
    m_pluginsMetaData[id] = metaData;
}
