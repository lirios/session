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
        qputenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/");
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_DIRS"))
        qputenv("XDG_CONFIG_DIRS", "/etc/xdg");
    if (qEnvironmentVariableIsEmpty("XDG_DESKTOP_PORTAL_DIR"))
        qputenv("XDG_DESKTOP_PORTAL_DIR", DATADIR "/xdg-desktop-portal/portals");

    // Environment
    qputenv("DESKTOP_SESSION", "Liri");
    qputenv("XDG_MENU_PREFIX", "liri-");
    qputenv("XDG_CURRENT_DESKTOP", "X-Liri");
    qputenv("XDG_SESSION_DESKTOP", "liri");

    // Set environment for the programs we will launch from here
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    qputenv("QT_QPA_PLATFORM", "wayland;xcb");
#else
    qputenv("QT_QPA_PLATFORM", "wayland");
#endif
    qputenv("QT_QPA_PLATFORMTHEME", "liri");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell");
    qputenv("QT_QUICK_CONTROLS_1_STYLE", "Flat");
    qputenv("QT_QUICK_CONTROLS_STYLE", "material");
    qputenv("QT_WAYLAND_DECORATION", "material");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    qputenv("XCURSOR_THEME", "Paper");
    qunsetenv("QT_SCALE_FACTOR");
    qunsetenv("QT_SCREEN_SCALE_FACTORS");
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

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("Liri session manager"));
    parser.addHelpOption();
    parser.addVersionOption();

#ifdef ENABLE_SYSTEMD
    // Disable systemd support
    QCommandLineOption noSystemdOption(
                QStringLiteral("no-systemd"),
                TR("Do not use systemd --user to bring up the session"));
    parser.addOption(noSystemdOption);
#endif

    // Modules to be disabled
    QCommandLineOption disableModulesOption(
                QStringLiteral("disable-modules"),
                TR("Comma separated list of modules to be disabled"),
                TR("list"));
    parser.addOption(disableModulesOption);

    // List modules
    QCommandLineOption listModulesOption(
            QStringLiteral("list-modules"),
            TR("List installed modules"));
    parser.addOption(listModulesOption);

    // Parse command line
    parser.process(app);

    // Create the session manager
    QSharedPointer<Session> session(new Session);

    // List modules and quit
    if (parser.isSet(listModulesOption)) {
        session->loadPlugins();

        const auto moduleNames = session->moduleNames();
        for (const auto &name : moduleNames)
            qInfo("%s", qPrintable(name));

        return 0;
    }

    // Arguments
    const QStringList disabledModulesList = parser.value(disableModulesOption).trimmed().split(QLatin1Char(','));
    const QStringList shellArgs = parser.positionalArguments();
#ifdef ENABLE_SYSTEMD
    const bool systemdSupport = !parser.isSet(noSystemdOption);
#else
    const bool systemdSupport = false;
#endif
    if (systemdSupport && parser.isSet(disableModulesOption))
        qWarning("The --disable-modules argument is not effective when systemd "
                 "is used to bring up the session");

    // Set systemd flag
    session->setSystemdEnabled(systemdSupport);

    // Disable incompatible modules
    QSet<QString> disabledModules = disabledModulesList.toSet();
    if (systemdSupport) {
        disabledModules.insert(QStringLiteral("services"));
        disabledModules.insert(QStringLiteral("shell"));
    } else {
        // Set shell arguments
        session->setModuleArguments(QStringLiteral("shell"), shellArgs);
    }

    // Disable modules
    for (const auto &name : disabledModules)
        session->disableModule(name);

    // We ship autostart entries with Hidden=true into $datadir/liri-session/systemd-user/autostart
    // for those entries that are replaced by units, so they are not started twice
    if (systemdSupport) {
        auto newValue = QDir::home().absoluteFilePath(QStringLiteral(".local/share/liri-session/systemd-user")).toUtf8();
        newValue.append(':');
        newValue.append(DATADIR "/liri-session/systemd-user");
        auto value = qgetenv("XDG_CONFIG_DIRS");
        if (value.isEmpty()) {
            value = newValue;
        } else {
            value.prepend(':');
            value.prepend(newValue);
        }
        qputenv("XDG_CONFIG_DIRS", value);
    }

    // Go
    QTimer::singleShot(0, &app, [session] {
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
