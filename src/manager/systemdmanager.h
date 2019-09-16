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

#ifndef SYSTEMDMANAGER_H
#define SYSTEMDMANAGER_H

#include <QObject>

class QProcessEnvironment;

class SystemdManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ isAvailable CONSTANT)
public:
    explicit SystemdManager(QObject *parent = nullptr);

    bool isAvailable() const;

    bool loadUnit(const QString &name);
    bool startUnit(const QString &name, const QString &mode);
    bool stopUnit(const QString &name, const QString &mode);

private:
    bool m_available = false;
};

#endif // SYSTEMDMANAGER_H
