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

#include "sessionmodule.h"
#include "sessionmodule_p.h"

namespace Liri {

// SessionModulePrivate

SessionModulePrivate::SessionModulePrivate(SessionModule *self)
    : q_ptr(self)
{
}

void SessionModulePrivate::setSystemdEnabled(bool enabled)
{
    Q_Q(SessionModule);

    if (systemd == enabled)
        return;

    systemd = enabled;
    emit q->systemdEnabledChanged();
}

// SessionModule

SessionModule::SessionModule(QObject *parent)
    : QObject(parent)
    , d_ptr(new SessionModulePrivate(this))
{
}

SessionModule::~SessionModule()
{
    delete d_ptr;
}

bool SessionModule::isSystemdEnabled() const
{
    Q_D(const SessionModule);
    return d->systemd;
}

} // namespace Liri
