// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "qmllauncher.h"

Q_LOGGING_CATEGORY(lcLauncher, "liri.session.launcher", QtInfoMsg)

QmlLauncher::QmlLauncher(QObject *parent)
    : QObject(parent)
{
}

void QmlLauncher::launchApplication(const QString &appId)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("LaunchApplication"));
    QVariantList args;
    args.append(appId);
    msg.setArguments(args);
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError())
            qCWarning(lcLauncher, "Failed to launch application: %s",
                      qPrintable(reply.error().message()));

        self->deleteLater();
    });
}

void QmlLauncher::launchDesktopFile(const QString &fileName)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("LaunchDesktopFile"));
    QVariantList args;
    args.append(fileName);
    msg.setArguments(args);
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError())
            qCWarning(lcLauncher, "Failed to launch application from desktop file: %s",
                      qPrintable(reply.error().message()));

        self->deleteLater();
    });
}

void QmlLauncher::launchCommand(const QString &command)
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("/io/liri/Launcher"),
                QStringLiteral("io.liri.Launcher"),
                QStringLiteral("LaunchCommand"));
    QVariantList args;
    args.append(command);
    msg.setArguments(args);
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError())
            qCWarning(lcLauncher, "Failed to launch command: %s",
                      qPrintable(reply.error().message()));

        self->deleteLater();
    });
}
