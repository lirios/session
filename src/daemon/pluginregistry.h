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

#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include <QObject>
#include <QHash>

typedef QHash<QString, QObject *> PluginsMap;

class PluginRegistry : public QObject
{
    Q_OBJECT
public:
    explicit PluginRegistry(QObject *parent = nullptr);

    PluginsMap plugins() const;

    bool hasPlugin(const QString &name) const;

    QVariantMap getMetaData(const QString &name) const;
    QObject *getInstance(const QString &name) const;

    QString getNameForInstance(QObject *instance) const;

    void discover();

private:
    PluginsMap m_plugins;
    QMap<QString, QVariantMap> m_pluginsMetaData;

    void addPlugin(QObject *instance, const QVariantMap &json);
};

#endif // PLUGINREGISTRY_H
