// SPDX-FileCopyrightText: 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logindtypes_p.h"

QDBusArgument &operator<<(QDBusArgument &argument, const DBusUserSession &userSession)
{
    argument.beginStructure();
    argument << userSession.id << userSession.objectPath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusUserSession &userSession)
{
    argument.beginStructure();
    argument >> userSession.id >> userSession.objectPath;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusSeat &seat)
{
    argument.beginStructure();
    argument << seat.id << seat.objectPath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusSeat &seat)
{
    argument.beginStructure();
    argument >> seat.id >> seat.objectPath;
    argument.endStructure();
    return argument;
}
