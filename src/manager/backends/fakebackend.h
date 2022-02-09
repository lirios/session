/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef FAKEBACKEND_H
#define FAKEBACKEND_H

#include "sessionbackend.h"

class FakeBackend : public SessionBackend
{
    Q_OBJECT
public:
    FakeBackend();

    QString name() const override;

    void setIdle(bool value) override;

    void inhibitIdle(const QString &who, const QString &why) override;
    void uninhibitIdle(int fd) override;

    void lockSession() override;
    void unlockSession() override;

    void locked();
    void unlocked();

    void switchToVt(quint32 vt) override;
};

#endif // FAKEBACKEND_H
