liri_add_module(Session
    DESCRIPTION
        "Session manager"
    SOURCES
        sessionmodule.cpp sessionmodule.h sessionmodule_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt6::Core
    PKGCONFIG_DEPENDENCIES
        Qt6Core
)

liri_finalize_module(Session)
