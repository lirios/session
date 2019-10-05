/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QMutex>

#include "fakebackend.h"
#include "logindbackend.h"
#include "sessionbackend.h"

static SessionBackend *s_backend = nullptr;

SessionBackend::SessionBackend(QObject *parent)
    : QObject(parent)
{
}

SessionBackend::~SessionBackend()
{
}

SessionBackend *SessionBackend::instance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    if (s_backend)
        return s_backend;
    if (LogindBackend::exists())
        s_backend = new LogindBackend();
    else
        s_backend = new FakeBackend();
    return s_backend;
}
