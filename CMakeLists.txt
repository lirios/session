cmake_minimum_required(VERSION 3.19)

project("Session"
    VERSION "0.1.0"
    DESCRIPTION "Session manager"
    LANGUAGES CXX C
)

## Only build on the appropriate platforms.
if(NOT (UNIX OR NOT CMAKE_SYSTEM_NAME MATCHES "Android"))
    message(NOTICE "Skipping the build as the condition \"UNIX OR NOT Android\" is not met.")
    return()
endif()

## ECM:
find_package(ECM 5.245.0 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

## Installation directories:
include(KDEInstallDirs)

## Compiler settings:
set(KDE_SKIP_NULLPTR_WARNINGS_SETTINGS TRUE)
include(KDECompilerSettings NO_POLICY_SCOPE)

## Enable keywords such as emit:
# TODO: Replace keywords such as emit with Q_EMIT
remove_definitions(-DQT_NO_KEYWORDS)

## CMake settings:
include(KDECMakeSettings)

## QML module:
include(ECMQmlModule)

## Disable use of C++ API deprecated in Qt 5.15
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

## Shared macros and functions:
if(NOT LIRI_LOCAL_ECM)
    find_package(LiriCMakeShared "2.0.99" REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH "${LCS_MODULE_PATH}")
endif()

## Liri specific setup common for all modules:
include(LiriSetup)

## Features:
include(features.cmake)

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
