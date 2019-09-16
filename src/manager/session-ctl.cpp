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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

static void doLogout()
{
    auto msg = QDBusMessage::createMethodCall(
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("/io/liri/SessionManager"),
                QStringLiteral("io.liri.SessionManager"),
                QStringLiteral("Logout"));
    msg.setAutoStartService(false);
    QDBusConnection::sessionBus().asyncCall(msg);
}

int main(int argc, char *argv[])
{
    // Application
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Session Control"));
    app.setApplicationVersion(QStringLiteral(VERSION));
    app.setOrganizationName(QStringLiteral("Liri"));
    app.setOrganizationDomain(QStringLiteral("liri.io"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("Liri session manager control"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Logout
    QCommandLineOption logoutOption(
                QStringLiteral("logout"),
                TR("Exit the session manager"));
    parser.addOption(logoutOption);

    // Parse command line
    parser.process(app);

    // Arguments
    const bool logout = parser.isSet(logoutOption);

    if (logout)
        doLogout();

    return app.exec();
}
