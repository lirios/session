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

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QProcess>

#include <LiriXdg/DesktopFile>

class Session;

typedef QMap<QString, QProcess *> ApplicationMap;
typedef QMutableMapIterator<QString, QProcess *> ApplicationMapIterator;

class SessionManager : public QObject
{
    Q_OBJECT
public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    bool registerWithDBus();

    void closeApplications();

public Q_SLOTS:
    bool LaunchApplication(const QString &appId);
    bool LaunchDesktopFile(const QString &fileName);
    bool LaunchCommand(const QString &command);
    void SetEnvironment(const QString &key, const QString &value);
    void Logout();

private:
    Session *m_session = nullptr;
    ApplicationMap m_apps;

    bool launchEntry(Liri::DesktopFile *entry);

private Q_SLOTS:
    void handleStandardOutput();
    void handleStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // SESSIONMANAGER_H
