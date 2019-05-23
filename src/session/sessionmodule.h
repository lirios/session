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

#ifndef LIRI_SESSIONMODULE_H
#define LIRI_SESSIONMODULE_H

#include <QtPlugin>
#include <QObject>

#include <LiriSession/lirisessionglobal.h>

#define LiriSessionModule_iid "io.liri.Session.Module/1.0"

namespace Liri {

class SessionModulePrivate;

class LIRISESSION_EXPORT SessionModule : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SessionModule)
public:
    enum StartupPhase {
        EarlyInitialization,
        Initialization,
        WindowManager,
        Applications
    };
    Q_ENUM(StartupPhase)

    explicit SessionModule(QObject *parent = nullptr);
    virtual ~SessionModule();

    virtual StartupPhase startupPhase() const = 0;

    QMap<QString, QString> environment() const;

    void setEnvironment(const QString &key, const QString &value);
    void unsetEnvironment(const QString &key);

    virtual bool start(const QStringList &args = QStringList()) = 0;
    virtual bool stop() = 0;

Q_SIGNALS:
    void environmentChangeRequested(const QString &key, const QString &value);

private:
    SessionModulePrivate *const d_ptr;
};

} // namespace Liri

Q_DECLARE_INTERFACE(Liri::SessionModule, LiriSessionModule_iid)

#endif // LIRI_SESSIONMODULE_H
