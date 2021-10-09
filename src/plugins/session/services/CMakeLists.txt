liri_add_plugin(services
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
    STATIC
)

liri_finalize_plugin(services)
