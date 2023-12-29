# Translations
file(GLOB TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/../../translations/*_*.ts")
qt6_add_translation(QM_FILES ${TRANSLATIONS})
install(FILES ${QM_FILES}
        DESTINATION "${KDE_INSTALL_DATADIR}/liri-session/translations")

# Git revision
include(LiriCreateGitHeadRevisionFile)
liri_create_git_head_revision_file("gitsha1.h")

if(NOT TARGET Liri::Xdg)
    find_package(Liri1Xdg REQUIRED)
endif()

qt6_add_dbus_adaptor(_dbus_sources dbus/io.liri.Launcher.xml dbus/processlauncher.h)
qt6_add_dbus_adaptor(_dbus_sources io.liri.SessionManager.xml dbus/sessionmanager.h)

set(_sources
    backends/fakebackend.cpp backends/fakebackend.h
    backends/logindbackend.cpp backends/logindbackend.h
    backends/logind/logind.cpp backends/logind/logind.h backends/logind/logind_p.h
    backends/logind/logindtypes.cpp backends/logind/logindtypes_p.h
    backends/sessionbackend.cpp backends/sessionbackend.h
    dbus/processlauncher.cpp dbus/processlauncher.h
    dbus/screensaver.cpp dbus/screensaver.h
    dbus/sessionmanager.cpp dbus/sessionmanager.h
    diagnostics.cpp diagnostics.h
    main.cpp
    pluginregistry.cpp pluginregistry.h
    session.cpp session.h
    systemdmanager.cpp systemdmanager.h
    utils.cpp utils.h
    ${QM_FILES}
    ${_dbus_sources}
)

add_executable(LiriSession ${_sources})

set_target_properties(LiriSession PROPERTIES OUTPUT_NAME liri-session)

target_compile_definitions(LiriSession
    PRIVATE
        LIRI_SESSION_VERSION="${PROJECT_VERSION}"
	DATADIR="${KDE_INSTALL_FULL_DATADIR}"
	PLUGINSDIR="${KDE_INSTALL_FULL_PLUGINSDIR}"
)

if(LIRI_SESSION_DEVELOPMENT_BUILD)
    target_compile_definitions(LiriSession PRIVATE DEVELOPMENT_BUILD)
endif()
if(LIRI_ENABLE_SYSTEMD)
    target_compile_definitions(LiriSession PRIVATE ENABLE_SYSTEMD)
endif()

target_link_libraries(LiriSession
    PRIVATE
        Qt6::Core
        Qt6::DBus
        Sigwatch
        Liri::Session
        Liri::SessionPrivate
        Liri::Xdg
        LiriSessionAutostartPlugin
        LiriSessionServicesPlugin
        LiriSessionShellPlugin
)

install(TARGETS LiriSession
	DESTINATION ${KDE_INSTALL_BINDIR}
)

###

add_executable(LiriSessionCtl session-ctl.cpp)

target_compile_definitions(LiriSessionCtl
    PRIVATE
        VERSION="${PROJECT_VERSION}"
)

target_link_libraries(LiriSessionCtl
    PRIVATE
        Qt6::Core
        Qt6::DBus
)

set(BINDIR "${KDE_INSTALL_FULL_BINDIR}")
configure_file(
    "liri.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/liri.desktop"
    @ONLY
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/liri.desktop"
    DESTINATION
        "${KDE_INSTALL_DATADIR}/wayland-sessions"
)
