# Defines
set(DEFINES "")

if(LIRI_ENABLE_SYSTEMD)
    list(APPEND DEFINES "ENABLE_SYSTEMD")
endif()

liri_add_module(Daemon
    DESCRIPTION
        "Daemon"
    SOURCES
        daemonmodule.cpp
        daemonmodule.h
    FORWARDING_HEADERS
        DaemonModule
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        ${DEFINES}
    PUBLIC_LIBRARIES
        Qt5::Core
        Qt5::DBus
    PKGCONFIG_DEPENDENCIES
        Qt5Core
        Qt5DBus
)
