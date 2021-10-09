if(NOT TARGET Liri::Qt5GSettings)
    find_package(Qt5GSettings REQUIRED)
endif()

liri_add_plugin(locale
    TYPE
        liri/daemon
    SOURCES
        plugin.cpp
        plugin.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Liri::Daemon
        Liri::Qt5GSettings
)

liri_finalize_plugin(locale)
