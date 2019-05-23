if(NOT TARGET Liri::Qt5GSettings)
    find_package(Qt5GSettings REQUIRED)
endif()

liri_add_plugin(locale
    TYPE
        liri/sessionmodules
    SOURCES
        plugin.cpp
        plugin.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt5::DBus
        Liri::Session
        Liri::Qt5GSettings
    STATIC
)
