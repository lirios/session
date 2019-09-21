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

#include "daemon.h"

#include <unistd.h>

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
#ifndef DEVELOPMENT_BUILD
    if (::getuid() == 0) {
        qWarning("Liri daemon cannot be run as root!");
        return 1;
    }
#endif

    // Application
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Daemon"));
    app.setApplicationVersion(QStringLiteral(LIRI_DAEMON_VERSION));
    app.setOrganizationName(QStringLiteral("Liri"));
    app.setOrganizationDomain(QStringLiteral("liri.io"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("Liri daemon"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Parse command line
    parser.process(app);

    // Create the daemon
    auto *daemon = Daemon::instance();

    // Go
    QTimer::singleShot(0, &app, [daemon] {
        // Initialize daemon
        if (!daemon->initialize()) {
            QCoreApplication::exit(1);
            return;
        }

        // Start all modules
        daemon->start();
    });

    return app.exec();
}
