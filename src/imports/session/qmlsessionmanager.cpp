// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "qmlsessionmanager.h"

Q_LOGGING_CATEGORY(lcSession, "liri.session.manager", QtInfoMsg)

/*
 * QmlSessionManager
 */

QmlSessionManager::QmlSessionManager(QObject *parent)
    : QObject(parent)
{
    // Emit when the session is locked or unlocked
    QDBusConnection::sessionBus().connect(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("Locked"),
                this, SIGNAL(sessionLocked()));
    QDBusConnection::sessionBus().connect(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("Unlocked"),
                this, SIGNAL(sessionUnlocked()));
}

bool QmlSessionManager::isIdle() const
{
    return m_idle;
}

void QmlSessionManager::setIdle(bool value)
{
    if (m_idle == value)
        return;

    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("SetIdle"));
    msg.setArguments(QVariantList() << value);
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [&](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError()) {
            qCWarning(lcSession, "Failed to toggle idle flag: %s",
                      qPrintable(reply.error().message()));
        } else {
            m_idle = value;
            emit idleChanged(m_idle);
        }

        self->deleteLater();
    });
}

void QmlSessionManager::lock()
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("Lock"));
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError())
            qCWarning(lcSession, "Failed to lock session: %s",
                      qPrintable(reply.error().message()));

        self->deleteLater();
    });
}

void QmlSessionManager::unlock()
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("Unlock"));
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError())
            qCWarning(lcSession, "Failed to unlock the session: %s",
                      qPrintable(reply.error().message()));

        self->deleteLater();
    });
}

void QmlSessionManager::setEnvironment(const QString &key, const QString &value)
{
    qputenv(key.toLocal8Bit().constData(), value.toLocal8Bit());

    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("SetEnvironment"));
    QVariantList args;
    args.append(key);
    args.append(value);
    msg.setArguments(args);
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;
        if (reply.isError())
            qCWarning(lcSession, "Failed to set environment: %s",
                      qPrintable(reply.error().message()));

        self->deleteLater();
    });
}
