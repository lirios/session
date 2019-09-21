# Translations
file(GLOB TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*_*.ts")
qt5_add_translation(QM_FILES ${TRANSLATIONS})
install(FILES ${QM_FILES}
        DESTINATION "${INSTALL_DATADIR}/liri-session/translations")

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

# Git revision
include(LiriCreateGitHeadRevisionFile)
liri_create_git_head_revision_file("gitsha1.h")

liri_add_executable(LiriSession
    OUTPUT_NAME
        "liri-session"
    SOURCES
        diagnostics.cpp
        diagnostics.h
        main.cpp
        pluginregistry.cpp
        pluginregistry.h
        session.cpp
        session.h
        sessionmanager.cpp
        sessionmanager.h
        translation.cpp
        translation.h
        utils.cpp
        utils.h
        sigwatch/sigwatch.cpp
        sigwatch/sigwatch.h
        sigwatch/sigwatch_p.h
        ${QM_FILES}
    DBUS_ADAPTOR_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/io.liri.SessionManager.xml"
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        LIRI_SESSION_VERSION="${PROJECT_VERSION}"
        ${DEFINES}
        PLUGINSDIR="${PLUGINSDIR}"
    LIBRARIES
        Qt5::Core
        Qt5::DBus
        Liri::Session
        autostart
        locale
        services
        shell
)

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
