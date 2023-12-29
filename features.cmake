# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

include(FeatureSummary)

## Find Qt:
set(QT_MIN_VERSION "6.6.0")
find_package(Qt6 "${QT_MIN_VERSION}"
    REQUIRED
    COMPONENTS
        Core
        DBus
        Xml
        Gui
        LinguistTools
)

#### Features

option(LIRI_SESSION_DEVELOPMENT_BUILD "Development build" OFF)
add_feature_info("Session::DevelopmentBuild" LIRI_SESSION_DEVELOPMENT_BUILD "Build for development")

option(LIRI_ENABLE_SYSTEMD "Enable systemd support" ON)
add_feature_info("Liri::Systemd" LIRI_ENABLE_SYSTEMD "Enable systemd support")

## Features summary:
if(NOT LIRI_SUPERBUILD)
    feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES)
endif()
