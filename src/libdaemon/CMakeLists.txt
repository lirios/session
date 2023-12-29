liri_add_module(Daemon
    DESCRIPTION
        "Daemon"
    SOURCES
        daemonmodule.cpp daemonmodule.h
    PUBLIC_LIBRARIES
        Qt6::Core
        Qt6::DBus
    PKGCONFIG_DEPENDENCIES
        Qt6Core
        Qt6DBus
)

if(LIRI_ENABLE_SYSTEMD)
    target_compile_definitions(Daemon PRIVATE ENABLE_SYSTEMD)
endif()

liri_finalize_module(Daemon)
