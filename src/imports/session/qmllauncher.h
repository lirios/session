// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LIRI_QML_SESSION_LAUNCHER_H
#define LIRI_QML_SESSION_LAUNCHER_H

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcLauncher)

class QmlLauncher : public QObject
{
    Q_OBJECT
public:
    QmlLauncher(QObject *parent = nullptr);

    Q_INVOKABLE void launchApplication(const QString &appId);
    Q_INVOKABLE void launchDesktopFile(const QString &fileName);
    Q_INVOKABLE void launchCommand(const QString &command);
};

#endif // LIRI_QML_SESSION_LAUNCHER_H
