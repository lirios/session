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

#ifndef PROCESSLAUNCHER_H
#define PROCESSLAUNCHER_H

#include <QObject>
#include <QProcess>

class Session;

class ProcessLauncher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.liri.Launcher")
public:
    explicit ProcessLauncher(QObject *parent = nullptr);
    ~ProcessLauncher();

    bool registerWithDBus();

    Q_SCRIPTABLE bool LaunchApplication(const QString &appId);
    Q_SCRIPTABLE bool LaunchDesktopFile(const QString &path, const QStringList &urls = QStringList());
    Q_SCRIPTABLE bool LaunchCommand(const QString &command);

    const QString serviceName = QStringLiteral("io.liri.Launcher");
    const QString objectPath = QStringLiteral("/io/liri/Launcher");

private:
    Session *m_session = nullptr;

    QString id(const QString &fileName) const;

private Q_SLOTS:
    void handleReadyReadStandardOutput();
    void handleReadyReadStandardError();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // PROCESSLAUNCHER_H
