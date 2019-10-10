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

#ifndef LIRI_SESSIONMODULE_P_H
#define LIRI_SESSIONMODULE_P_H

#include <LiriSession/SessionModule>

namespace Liri {

class LIRISESSION_EXPORT SessionModulePrivate
{
    Q_DECLARE_PUBLIC(SessionModule)
public:
    SessionModulePrivate(SessionModule *self);

    void setSystemdEnabled(bool enabled);

    static SessionModulePrivate *get(SessionModule *module) { return module->d_func(); }

    bool systemd = false;

protected:
    SessionModule *q_ptr = nullptr;
};

} // namespace Liri

#endif // LIRI_SESSIONMODULE_P_H
