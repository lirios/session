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

#include <QDBusConnection>
#include <QDBusInterface>

#include <Qt5GSettings/QGSettings>

#include "plugin.h"

const QString languageKey = QStringLiteral("language");
const QString regionKey = QStringLiteral("region");

const QString interfaceName = QStringLiteral("org.freedesktop.locale1");
const QString objectPath = QStringLiteral("/org/freedesktop/locale1");

LocalePlugin::LocalePlugin(QObject *parent)
    : Liri::SessionModule(parent)
{
    getSystemLocale();

    m_settings = new QtGSettings::QGSettings(
                QStringLiteral("io.liri.session.locale"),
                QStringLiteral("/io/liri/session/locale/"),
                this);
    connect(m_settings, &QtGSettings::QGSettings::settingChanged,
            this, &LocalePlugin::handleSettingChanged);
}

Liri::SessionModule::StartupPhase LocalePlugin::startupPhase() const
{
    return Initialization;
}

bool LocalePlugin::start(const QStringList &args)
{
    Q_UNUSED(args)

    handleSettingChanged(languageKey);
    handleSettingChanged(regionKey);

    return true;
}

bool LocalePlugin::stop()
{
    return true;
}

void LocalePlugin::setEnvironment(const QString &key, const QString &value)
{
    Q_EMIT environmentChangeRequested(key, value);
}

void LocalePlugin::getSystemLocale()
{
    auto interface = new QDBusInterface(
                interfaceName, objectPath, interfaceName,
                QDBusConnection::systemBus(), this);

    auto locale = qvariant_cast<QStringList>(interface->property("Locale"));
    for (const auto &entry : locale) {
        auto nameValue = entry.split(QLatin1Char('='));
        if (nameValue.length() == 2)
            m_systemLocale[nameValue.at(0)] = nameValue.at(1);
    }
}

void LocalePlugin::handleSettingChanged(const QString &key)
{
    if (key == languageKey) {
        m_language = m_settings->value(languageKey).toString();
        if (m_language.isEmpty())
            m_language = m_systemLocale[QStringLiteral("LANG")];

        setEnvironment(QStringLiteral("LANG"), m_language);
        setEnvironment(QStringLiteral("LANGUAGE"), m_language);
        setEnvironment(QStringLiteral("LC_MESSAGES"), m_language);
    } else if (key == regionKey) {
        m_region = m_settings->value(regionKey).toString();
        if (m_region.isEmpty())
            m_region = m_systemLocale[QStringLiteral("LANG")];

        setEnvironment(QStringLiteral("LC_CTYPE"), m_region);
        setEnvironment(QStringLiteral("LC_NUMERIC"), m_region);
        setEnvironment(QStringLiteral("LC_TIME"), m_region);
        setEnvironment(QStringLiteral("LC_COLLATE"), m_region);
        setEnvironment(QStringLiteral("LC_MONETARY"), m_region);
        setEnvironment(QStringLiteral("LC_PAPER"), m_region);
        setEnvironment(QStringLiteral("LC_NAME"), m_region);
        setEnvironment(QStringLiteral("LC_ADDRESS"), m_region);
        setEnvironment(QStringLiteral("LC_TELEPHONE"), m_region);
        setEnvironment(QStringLiteral("LC_MEASUREMENT"), m_region);
        setEnvironment(QStringLiteral("LC_IDENTIFICATION"), m_region);
    }
}
