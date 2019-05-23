if(NOT TARGET Liri::Xdg)
    find_package(Liri1Xdg REQUIRED)
endif()

liri_add_plugin(autostart
    TYPE
        liri/sessionmodules
    SOURCES
        plugin.cpp
        plugin.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Liri::Session
        Liri::Xdg
    STATIC
)
