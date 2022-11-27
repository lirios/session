// SPDX-FileCopyrightText: 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINDTYPES_P_H
#define LOGINDTYPES_P_H

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QVector>

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

class DBusUserSession
{
public:
    QString id;
    QDBusObjectPath objectPath;
};
Q_DECLARE_METATYPE(DBusUserSession)

typedef QVector<DBusUserSession> DBusUserSessionVector;
Q_DECLARE_METATYPE(DBusUserSessionVector)

class DBusSeat
{
public:
    QString id;
    QDBusObjectPath objectPath;
};
Q_DECLARE_METATYPE(DBusSeat)

QDBusArgument &operator<<(QDBusArgument &argument, const DBusUserSession &userSession);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusUserSession &userSession);

QDBusArgument &operator<<(QDBusArgument &argument, const DBusSeat &seat);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusSeat &seat);

#endif // LOGINDTYPES_P_H
