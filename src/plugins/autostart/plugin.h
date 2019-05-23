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

#ifndef AUTOSTARTPLUGIN_H
#define AUTOSTARTPLUGIN_H

#include <QLoggingCategory>
#include <QObject>
#include <QProcess>

#include <LiriXdg/DesktopFile>
#include <LiriSession/SessionModule>

Q_DECLARE_LOGGING_CATEGORY(lcSession)

class AutostartPlugin : public Liri::SessionModule
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LiriSessionModule_iid FILE "plugin.json")
    Q_INTERFACES(Liri::SessionModule)
public:
    explicit AutostartPlugin(QObject *parent = nullptr);

    StartupPhase startupPhase() const override;

    bool start(const QStringList &args = QStringList()) override;
    bool stop() override;

private:
    QVector<QProcess *> m_processes;

    bool launchEntry(const Liri::DesktopFile &entry);

private Q_SLOT:
    void handleStandardOutput();
    void handleStandardError();
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // AUTOSTARTPLUGIN_H
