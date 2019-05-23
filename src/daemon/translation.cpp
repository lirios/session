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

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

#include "translation.h"

void loadQtTranslations()
{
#ifndef QT_NO_TRANSLATION
    QString locale = QLocale::system().name();

    // Load Qt translations
    QTranslator *qtTranslator = new QTranslator(QCoreApplication::instance());
    if (qtTranslator->load(QStringLiteral("qt_%1").arg(locale), QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        qApp->installTranslator(qtTranslator);
    } else {
        delete qtTranslator;
    }
#endif
}

void loadAppTranslations()
{
#ifndef QT_NO_TRANSLATION
    QString locale = QLocale::system().name();

    // Find the translations directory
    const QString path = QLatin1String("liri-session/translations");
    const QString translationsDir =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   path,
                                   QStandardPaths::LocateDirectory);

    // Load shell translations
    QTranslator *appTranslator = new QTranslator(QCoreApplication::instance());
    if (appTranslator->load(QStringLiteral("%1/session_%2").arg(translationsDir, locale))) {
        QCoreApplication::installTranslator(appTranslator);
    } else if (locale == QLatin1String("C") ||
               locale.startsWith(QLatin1String("en"))) {
        // English is the default, it's translated anyway
        delete appTranslator;
    }
#endif
}
