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

#ifndef LIRI_DAEMONMODULE_H
#define LIRI_DAEMONMODULE_H

#include <QtPlugin>
#include <QObject>

#include <LiriDaemon/liridaemonglobal.h>

#define LiriDaemonModule_iid "io.liri.Daemon.Module/1.0"

namespace Liri {

class DaemonModulePrivate;

class LIRIDAEMON_EXPORT DaemonModule : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DaemonModule)
public:
    explicit DaemonModule(QObject *parent = nullptr);
    virtual ~DaemonModule();

    virtual void start() = 0;
    virtual void stop() = 0;

Q_SIGNALS:
    void moduleDeleted();

private:
    DaemonModulePrivate *const d_ptr;
};

} // namespace Liri

Q_DECLARE_INTERFACE(Liri::DaemonModule, LiriDaemonModule_iid)

#endif // LIRI_DAEMONMODULE_H
