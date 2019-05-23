cmake_minimum_required(VERSION 3.10.0)

project("Session"
    VERSION "0.1.0"
    DESCRIPTION "Session manager"
    LANGUAGES CXX C
)

## Shared macros and functions:
if(NOT LIRI_LOCAL_ECM)
    find_package(LiriCMakeShared "1.0.0" REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH "${LCS_MODULE_PATH}")
endif()

## Set minimum versions required:
set(QT_MIN_VERSION "5.10.0")

## Liri specific setup common for all modules:
include(LiriSetup)

## Features:
option(LIRI_SESSION_DEVELOPMENT_BUILD "Development build" OFF)
add_feature_info("Session::DevelopmentBuild" LIRI_SESSION_DEVELOPMENT_BUILD "Build for development")

## Variables:
set(INSTALL_SYSTEMDUSERUNITDIR "${INSTALL_LIBDIR}/systemd/user" CACHE PATH "systemd user unit directory [PREFIX/lib/systemd/user]")

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
add_subdirectory(src/session)
add_subdirectory(src/plugins/autostart)
add_subdirectory(src/plugins/locale)
add_subdirectory(src/plugins/shell)
