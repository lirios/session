# Defines
set(DEFINES "")
if(LIRI_SESSION_DEVELOPMENT_BUILD)
    list(APPEND DEFINES "DEVELOPMENT_BUILD")
endif()

# Absolute paths
if(IS_ABSOLUTE "${INSTALL_PLUGINSDIR}")
    set(PLUGINSDIR "${INSTALL_PLUGINSDIR}")
else()
    set(PLUGINSDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_PLUGINSDIR}")
endif()

liri_add_executable(LiriDaemon
    OUTPUT_NAME
        "liri-daemon"
    SOURCES
        daemon.cpp
        daemon.h
        daemoninterface.cpp
        daemoninterface.h
        main.cpp
        pluginregistry.cpp
        pluginregistry.h
    DBUS_ADAPTOR_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/io.liri.Daemon.xml"
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        LIRI_DAEMON_VERSION="${PROJECT_VERSION}"
        ${DEFINES}
        PLUGINSDIR="${PLUGINSDIR}"
    INSTALL_DIRECTORY
        "${INSTALL_LIBEXECDIR}"
    LIBRARIES
        Qt5::Core
        Qt5::DBus
        Sigwatch
        Liri::Daemon
)
