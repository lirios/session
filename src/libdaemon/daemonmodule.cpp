/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QMap>

#include "daemonmodule.h"

namespace Liri {

class DaemonModulePrivate
{
public:
    DaemonModulePrivate() {}

    QString name;
};

DaemonModule::DaemonModule(QObject *parent)
    : QObject(parent)
    , d_ptr(new DaemonModulePrivate)
{
}

DaemonModule::~DaemonModule()
{
    emit moduleDeleted();
    delete d_ptr;
}

QString DaemonModule::name() const
{
    Q_D(const DaemonModule);
    return d->name;
}

void DaemonModule::setName(const QString &name)
{
    Q_D(DaemonModule);

    if (d->name == name)
        return;

    d->name = name;
}

} // namespace Liri
