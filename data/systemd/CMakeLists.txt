set(ABSOLUTE_BINDIR "${KDE_INSTALL_FULL_BINDIR}")
set(ABSOLUTE_LIBEXECDIR "${KDE_INSTALL_FULL_LIBEXECDIR}")

configure_file(
    "liri-daemon-locale.service.in"
    "${CMAKE_CURRENT_BINARY_DIR}/liri-daemon-locale.service"
    @ONLY
)
configure_file(
    "liri-session-shutdown.service.in"
    "${CMAKE_CURRENT_BINARY_DIR}/liri-session-shutdown.service"
    @ONLY
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/liri-daemon-locale.service"
        liri-daemons.target
        liri-services.target
        liri-session-pre.target
        "${CMAKE_CURRENT_BINARY_DIR}/liri-session-shutdown.service"
        liri-session.target
    DESTINATION
        "${KDE_INSTALL_SYSTEMDUSERUNITDIR}"
)
