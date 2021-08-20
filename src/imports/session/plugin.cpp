// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QtQml>

#include "qmllauncher.h"
#include "qmlsessionmanager.h"

class SessionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(strcmp(uri, "Liri.Session") == 0);

        // @uri Liri.Session
        qmlRegisterSingletonType<QmlLauncher>(uri, 1, 0, "Launcher",
                                              [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new QmlLauncher();
        });
        qmlRegisterSingletonType<QmlSessionManager>(uri, 1, 0, "SessionManager",
                                                    [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new QmlSessionManager();
        });
    }
};

#include "plugin.moc"
