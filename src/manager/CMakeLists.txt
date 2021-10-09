# Translations
file(GLOB TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/../../translations/*_*.ts")
qt5_add_translation(QM_FILES ${TRANSLATIONS})
install(FILES ${QM_FILES}
        DESTINATION "${INSTALL_DATADIR}/liri-session/translations")

# Defines
set(DEFINES "")

if(LIRI_SESSION_DEVELOPMENT_BUILD)
    list(APPEND DEFINES "DEVELOPMENT_BUILD")
endif()

# Absolute paths
if(IS_ABSOLUTE "${INSTALL_DATADIR}")
    set(DATADIR "${INSTALL_DATADIR}")
else()
    set(DATADIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_DATADIR}")
endif()
if(IS_ABSOLUTE "${INSTALL_PLUGINSDIR}")
    set(PLUGINSDIR "${INSTALL_PLUGINSDIR}")
else()
    set(PLUGINSDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_PLUGINSDIR}")
endif()

# Git revision
include(LiriCreateGitHeadRevisionFile)
liri_create_git_head_revision_file("gitsha1.h")

if(LIRI_ENABLE_SYSTEMD)
    list(APPEND DEFINES "ENABLE_SYSTEMD")
endif()

if(NOT TARGET Liri::Logind)
    find_package(Liri1Logind)
endif()
if(NOT TARGET Liri::Xdg)
    find_package(Liri1Xdg REQUIRED)
endif()

liri_add_executable(LiriSession
    OUTPUT_NAME
        "liri-session"
    SOURCES
        backends/fakebackend.cpp
        backends/fakebackend.h
        backends/logindbackend.cpp
        backends/logindbackend.h
        backends/sessionbackend.cpp
        backends/sessionbackend.h
        dbus/processlauncher.cpp
        dbus/processlauncher.h
        dbus/screensaver.cpp
        dbus/screensaver.h
        dbus/sessionmanager.cpp
        dbus/sessionmanager.h
        diagnostics.cpp
        diagnostics.h
        main.cpp
        pluginregistry.cpp
        pluginregistry.h
        session.cpp
        session.h
        systemdmanager.cpp
        systemdmanager.h
        utils.cpp
        utils.h
        ${QM_FILES}
    DBUS_ADAPTOR_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/dbus/io.liri.Launcher.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/io.liri.SessionManager.xml"
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        LIRI_SESSION_VERSION="${PROJECT_VERSION}"
        ${DEFINES}
        DATADIR="${DATADIR}"
        PLUGINSDIR="${PLUGINSDIR}"
    LIBRARIES
        Qt5::Core
        Qt5::DBus
        Sigwatch
        Liri::Session
        Liri::SessionPrivate
        Liri::Logind
        Liri::Xdg
        autostart
        services
        shell
)

liri_finalize_executable(LiriSession)

liri_add_executable(LiriSessionCtl
    OUTPUT_NAME
        "liri-session-ctl"
    SOURCES
        session-ctl.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        VERSION="${PROJECT_VERSION}"
    LIBRARIES
        Qt5::Core
        Qt5::DBus
)

liri_finalize_executable(LiriSessionCtl)

if(IS_ABSOLUTE "${INSTALL_BINDIR}")
    set(BINDIR "${INSTALL_BINDIR}")
else()
    set(BINDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}")
endif()
configure_file(
    "liri.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/liri.desktop"
    @ONLY
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/liri.desktop"
    DESTINATION
        "${INSTALL_DATADIR}/wayland-sessions"
)
