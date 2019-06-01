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
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QSharedPointer>
#include <QTimer>

#include "session.h"
#include "translation.h"

#include <unistd.h>

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

static void setupEnvironment()
{
    // Set defaults
    if (qEnvironmentVariableIsEmpty("XDG_DATA_HOME"))
        qputenv("XDG_DATA_HOME", QDir::home().absoluteFilePath(QStringLiteral(".local/share")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_HOME"))
        qputenv("XDG_CONFIG_HOME", QDir::home().absoluteFilePath(QStringLiteral(".config")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_CACHE_HOME"))
        qputenv("XDG_CACHE_HOME", QDir::home().absoluteFilePath(QStringLiteral(".cache")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_DATA_DIRS"))
        qputenv("XDG_DATA_DIRS", QByteArrayLiteral("/usr/local/share/:/usr/share/"));
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_DIRS"))
        qputenv("XDG_CONFIG_DIRS", QByteArrayLiteral("/etc/xdg"));

    // Environment
    qputenv("DESKTOP_SESSION", QByteArrayLiteral("Liri"));
    qputenv("XDG_MENU_PREFIX", QByteArrayLiteral("liri-"));
    qputenv("XDG_CURRENT_DESKTOP", QByteArrayLiteral("X-Liri"));
    qputenv("XDG_SESSION_DESKTOP", QByteArrayLiteral("liri"));
}

int main(int argc, char *argv[])
{
#ifndef DEVELOPMENT_BUILD
    if (::getuid() == 0) {
        qWarning("Liri session manager cannot be run as root!");
        return 1;
    }
#endif

    // Setup the environment
    setupEnvironment();

    // Application
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Session"));
    app.setApplicationVersion(QStringLiteral(LIRI_SESSION_VERSION));
    app.setOrganizationName(QStringLiteral("Liri"));
    app.setOrganizationDomain(QStringLiteral("liri.io"));

    // Load translations
    loadQtTranslations();
    loadAppTranslations();

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("Liri session manager"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Modules to be disabled
    QCommandLineOption disableModulesOption(
                QStringLiteral("disable-modules"),
                TR("Comma separated list of modules to be disabled"),
                TR("list"));
    parser.addOption(disableModulesOption);

    // Parse command line
    parser.process(app);

    // Arguments
    const QStringList disabledModules = parser.value(disableModulesOption).trimmed().split(QLatin1Char(','));
    const QStringList shellArgs = parser.positionalArguments();

    // Create the session manager
    QSharedPointer<Session> session(new Session);

    // Set shell arguments
    session->setModuleArguments(QStringLiteral("shell"), shellArgs);

    // Disable modules
    for (const auto &name : disabledModules)
        session->disableModule(name);

    // Go
    QTimer::singleShot(0, [session] {
        // A D-Bus session is required
        if (session->requireDBusSession())
            return;

        // Initialize session manager
        if (!session->initialize()) {
            QCoreApplication::exit(1);
            return;
        }

        // Start all services
        if (!session->start()) {
            QCoreApplication::exit(1);
            return;
        }
    });

    return app.exec();
}
