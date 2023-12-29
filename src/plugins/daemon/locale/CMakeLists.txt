if(NOT TARGET Liri::Qt6GSettings)
    find_package(Qt6GSettings REQUIRED)
endif()

qt6_add_plugin(LiriDaemonLocalePlugin
    STATIC
    CLASS_NAME LocalePlugin
    MANUAL_FINALIZATION
    plugin.cpp plugin.h
)

set_target_properties(LiriDaemonLocalePlugin PROPERTIES OUTPUT_NAME locale)

target_link_libraries(LiriDaemonLocalePlugin
    PRIVATE
        Liri::Daemon
        Liri::Qt6GSettings
)

qt6_finalize_target(LiriDaemonLocalePlugin)

install(
    TARGETS LiriDaemonLocalePlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/liri/daemon
)
