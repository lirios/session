# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(SessionQmlPlugin
    URI Liri.Session
    VERSION 1.0
    CLASS_NAME SessionPlugin
    NO_GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(SessionQmlPlugin
    PRIVATE
        plugin.cpp
        qmllauncher.cpp qmllauncher.h
        qmlsessionmanager.cpp qmlsessionmanager.h
)

target_link_libraries(SessionQmlPlugin
    PRIVATE
        Qt6::Core
        Qt6::DBus
        Qt6::Qml
)

ecm_finalize_qml_module(SessionQmlPlugin)
