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

#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QElapsedTimer>
#include <QObject>
#include <QMap>

struct InhibitEntry {
    QString who;
    QString why;
};

class ScreenSaver : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ScreenSaver")
public:
    explicit ScreenSaver(QObject *parent = nullptr);
    ~ScreenSaver();

    bool registerWithDBus();

    Q_SCRIPTABLE bool GetActive();
    Q_SCRIPTABLE bool SetActive(bool state);

    Q_SCRIPTABLE uint GetActiveTime();
    Q_SCRIPTABLE uint GetSessionIdleTime();

    Q_SCRIPTABLE void SimulateUserActivity();

    Q_SCRIPTABLE uint Inhibit(const QString &appName, const QString &reason);
    Q_SCRIPTABLE void UnInhibit(uint cookie);

    Q_SCRIPTABLE void Lock();

    Q_SCRIPTABLE uint Throttle(const QString &appName, const QString &reason);
    Q_SCRIPTABLE void UnThrottle(uint cookie);

    const QString serviceName = QStringLiteral("org.freedesktop.ScreenSaver");
    const QString objectPath = QStringLiteral("/ScreenSaver");

Q_SIGNALS:
    Q_SCRIPTABLE void ActiveChanged(bool in);

private:
    bool m_active = false;
    QMap<uint, InhibitEntry> m_inhibit;
    QMap<uint, int> m_inhibitFd;
    QElapsedTimer m_elapsedTimer;

private Q_SLOTS:
    void handleLock();
    void handleUnlock();
    void handleInhibited(const QString &who, const QString &why, int fd);
};

#endif // SCREENSAVER_H
