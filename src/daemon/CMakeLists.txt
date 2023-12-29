qt6_add_dbus_adaptor(_dbus_sources io.liri.Daemon.xml daemoninterface.h)
set(_sources
    daemon.cpp daemon.h
    daemoninterface.cpp daemoninterface.h
    main.cpp
    pluginregistry.cpp pluginregistry.h
    ${_dbus_sources}
)
add_executable(LiriDaemon ${_sources})

set_target_properties(LiriDaemon PROPERTIES OUTPUT_NAME liri-daemon)

target_compile_definitions(LiriDaemon
    PRIVATE
        LIRI_DAEMON_VERSION="${PROJECT_VERSION}"
	PLUGINSDIR="${KDE_INSTALL_FULL_PLUGINSDIR}"
)

if(LIRI_SESSION_DEVELOPMENT_BUILD)
    target_compile_definitions(LiriDaemon PRIVATE DEVELOPMENT_BUILD)
endif()
if(LIRI_ENABLE_SYSTEMD)
    target_compile_definitions(LiriDaemon PRIVATE ENABLE_SYSTEMD)
endif()

target_link_libraries(LiriDaemon
    PRIVATE
        Qt6::Core
        Qt6::DBus
        Sigwatch
        Liri::Daemon
)

install(TARGETS LiriDaemon
        DESTINATION ${KDE_INSTALL_LIBEXECDIR}
)
