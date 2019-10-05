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

#ifndef LOGINDBACKEND_H
#define LOGINDBACKEND_H

#include "sessionbackend.h"

class LogindBackend : public SessionBackend
{
    Q_OBJECT
public:
    LogindBackend();
    ~LogindBackend();

    QString name() const;

    void setIdle(bool value);

    void lockSession();
    void unlockSession();

    void switchToVt(quint32 vt);

    static bool exists();

private:
    int m_powerButtonFd = -1;
    int m_inhibitFd = -1;

private Q_SLOTS:
    void setupInhibitors();
    void handleConnectedChanged(bool connected);
    void handleInhibited(const QString &who, const QString &why, int fd);
    void handleUninhibited(int fd);
    void prepareForSleep(bool arg);
    void prepareForShutdown(bool arg);
    void devicePaused(quint32 devMajor, quint32 devMinor, const QString &type);
    void handleSessionLocked();
    void handleSessionUnlocked();
};

#endif // LOGINDBACKEND_H
