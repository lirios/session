if(NOT TARGET Liri::Xdg)
    find_package(Liri1Xdg)
endif()

liri_add_executable(LiriSessionAutostartGenerator
    OUTPUT_NAME
        "liri-session-autostart-generator"
    SOURCES
        autostart-generator.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    INSTALL_DIRECTORY
        "${INSTALL_SYSTEMDUSERGENERATORSDIR}"
    LIBRARIES
        Qt5::Core
        Liri::Xdg
)
