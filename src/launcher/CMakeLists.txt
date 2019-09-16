if(NOT TARGET Liri::Xdg)
    find_package(Liri1Xdg REQUIRED)
endif()

liri_add_executable(LiriLauncher
    OUTPUT_NAME
        "liri-launcher"
    SOURCES
        main.cpp
        processlauncher.cpp
        processlauncher.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        VERSION="${PROJECT_VERSION}"
    INSTALL_DIRECTORY
        "${INSTALL_LIBEXECDIR}"
    LIBRARIES
        Qt5::Core
        Qt5::DBus
        Liri::Xdg
)

if(ENABLE_SYSTEMD)
    set(systemd "SystemdUnit=liri-launcher.service")
endif()
if(IS_ABSOLUTE "${INSTALL_LIBEXECDIR}")
    set(LIBEXECDIR "${INSTALL_LIBEXECDIR}")
else()
    set(LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBEXECDIR}")
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/io.liri.Launcher.service.in"
               "${CMAKE_CURRENT_BINARY_DIR}/io.liri.Launcher.service"
               @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/io.liri.Launcher.service"
        DESTINATION "${INSTALL_DATADIR}/dbus-1/services")
