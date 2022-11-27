// SPDX-FileCopyrightText: 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Liri API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusServiceWatcher>
#include <QLoggingCategory>
#include <QVector>

#include "logindtypes_p.h"

Q_DECLARE_LOGGING_CATEGORY(gLcLogind)

class Logind;

class LogindPrivate
{
    Q_DECLARE_PUBLIC(Logind)
public:
    explicit LogindPrivate(Logind *qq);

    void _q_serviceRegistered();
    void _q_serviceUnregistered();
    void _q_sessionPropertiesChanged();

    void checkServiceRegistration();

    QDBusConnection bus;
    QDBusServiceWatcher *watcher = nullptr;
    bool isConnected = false;
    bool hasSessionControl = false;
    QString sessionPath;
    bool sessionActive = false;
    int vt = -1;
    QString seat;
    QVector<int> inhibitFds;

protected:
    Logind *q_ptr;

private:
    bool getSessionById(const QString &sessionId, QDBusObjectPath &path) const;
    bool getSessionByPid(QDBusObjectPath &path) const;
    bool getUserSession(DBusUserSession &session) const;
    QString getSessionId(const QString &sessionPath) const;
    QString getSessionType(const QString &sessionId, const QString &sessionPath) const;
    QString getSessionState(const QString &sessionId, const QString &sessionPath) const;

    void getSessionActive();
    void getVirtualTerminal();
    void getSeat();
};
