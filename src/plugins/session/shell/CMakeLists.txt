if(IS_ABSOLUTE "${INSTALL_LIBEXECDIR}")
    set(LIBEXECDIR "${INSTALL_LIBEXECDIR}")
else()
    set(LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBEXECDIR}")
endif()

liri_add_plugin(shell
    TYPE
        liri/sessionmodules
    SOURCES
        plugin.cpp
        plugin.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        LIBEXECDIR="${LIBEXECDIR}"
    LIBRARIES
        Qt5::DBus
        Liri::Session
    STATIC
)

liri_finalize_plugin(shell)
