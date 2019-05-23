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

#ifndef LOCALEPLUGIN_H
#define LOCALEPLUGIN_H

#include <QLoggingCategory>
#include <QObject>

#include <LiriSession/SessionModule>

Q_DECLARE_LOGGING_CATEGORY(lcSession)

namespace QtGSettings {
class QGSettings;
} // namespace QtGSettings

class LocalePlugin : public Liri::SessionModule
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LiriSessionModule_iid FILE "plugin.json")
    Q_INTERFACES(Liri::SessionModule)
public:
    explicit LocalePlugin(QObject *parent = nullptr);

    StartupPhase startupPhase() const override;

    bool start(const QStringList &args = QStringList()) override;
    bool stop() override;

private:
    QtGSettings::QGSettings *m_settings = nullptr;
    QString m_language;
    QString m_region;
    QMap<QString, QString> m_systemLocale;

    void setEnvironment(const QString &key, const QString &value);
    void getSystemLocale();

private Q_SLOTS:
    void handleSettingChanged(const QString &key);
};

#endif // LOCALEPLUGIN_H
