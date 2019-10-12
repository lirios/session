if(IS_ABSOLUTE "${INSTALL_BINDIR}")
    set(ABSOLUTE_BINDIR "${INSTALL_BINDIR}")
else()
    set(ABSOLUTE_BINDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}")
endif()
if(IS_ABSOLUTE "${INSTALL_LIBEXECDIR}")
    set(ABSOLUTE_LIBEXECDIR "${INSTALL_LIBEXECDIR}")
else()
    set(ABSOLUTE_LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBEXECDIR}")
endif()
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
        "${INSTALL_SYSTEMDUSERUNITDIR}"
)
