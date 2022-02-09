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

#ifndef SESSIONBACKEND_H
#define SESSIONBACKEND_H

#include <QObject>

class SessionBackend : public QObject
{
    Q_OBJECT
public:
    SessionBackend(QObject *parent = nullptr);
    virtual ~SessionBackend();

    virtual QString name() const = 0;

    virtual void setIdle(bool value) = 0;

    virtual void inhibitIdle(const QString &who, const QString &why) = 0;
    virtual void uninhibitIdle(int fd) = 0;

    virtual void lockSession() = 0;
    virtual void unlockSession() = 0;

    virtual void switchToVt(quint32 vt) = 0;

    static SessionBackend *instance();

Q_SIGNALS:
    void inhibited(const QString &who, const QString &why, int fd);
    void sessionLocked();
    void sessionUnlocked();
    void shutdownRequested();
};

#endif // SESSIONBACKEND_H
