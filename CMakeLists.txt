cmake_minimum_required(VERSION 3.10.0)

project("Session"
    VERSION "0.1.0"
    DESCRIPTION "Session manager"
    LANGUAGES CXX C
)

## Shared macros and functions:
if(NOT LIRI_LOCAL_ECM)
    find_package(LiriCMakeShared "2.0.0" REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH "${LCS_MODULE_PATH}")
endif()

## Set minimum versions required:
set(QT_MIN_VERSION "5.10.0")

## Liri specific setup common for all modules:
include(LiriSetup)

## Only build on the appropriate platforms.
if(NOT UNIX OR ANDROID)
    message(NOTICE "Skipping the build as the condition \"UNIX OR ANDROID\" is not met.")
    return()
endif()

## Features:
option(LIRI_SESSION_DEVELOPMENT_BUILD "Development build" OFF)
add_feature_info("Session::DevelopmentBuild" LIRI_SESSION_DEVELOPMENT_BUILD "Build for development")
option(LIRI_ENABLE_SYSTEMD "Enable systemd support" ON)
add_feature_info("Liri::Systemd" LIRI_ENABLE_SYSTEMD "Enable systemd support")

## Variables:
if(LIRI_ENABLE_SYSTEMD)
    set(INSTALL_SYSTEMDUSERUNITDIR "${INSTALL_LIBDIR}/systemd/user" CACHE PATH "systemd user unit directory [PREFIX/lib/systemd/user]")
    set(INSTALL_SYSTEMDUSERGENERATORSDIR "${INSTALL_LIBDIR}/systemd/user-generators" CACHE PATH "systemd user generators directory [PREFIX/lib/systemd/user-generators]")
endif()

## Find Qt 5:
find_package(Qt5 "${QT_MIN_VERSION}"
    CONFIG REQUIRED
    COMPONENTS
        Core
        DBus
	Xml
	Gui
        LinguistTools
)

## Add subdirectories:
add_subdirectory(data/menu)
add_subdirectory(data/settings)
add_subdirectory(src/daemon)
add_subdirectory(src/imports/session)
add_subdirectory(src/manager)
add_subdirectory(src/libdaemon)
add_subdirectory(src/libsession)
add_subdirectory(src/libsigwatch)
add_subdirectory(src/plugins/daemon/locale)
add_subdirectory(src/plugins/session/autostart)
add_subdirectory(src/plugins/session/services)
add_subdirectory(src/plugins/session/shell)
if(LIRI_ENABLE_SYSTEMD)
    add_subdirectory(data/systemd)
    add_subdirectory(data/systemd/autostart)
endif()
