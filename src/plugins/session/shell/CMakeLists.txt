qt6_add_plugin(LiriSessionShellPlugin
    STATIC
    CLASS_NAME ShellPlugin
    MANUAL_FINALIZATION
    plugin.cpp plugin.h
)

#set_target_properties(LiriSessionShellPlugin PROPERTIES OUTPUT_NAME shell)

target_compile_definitions(LiriSessionShellPlugin PRIVATE LIBEXECDIR="${KDE_INSTALL_FULL_LIBEXECDIR}")

target_link_libraries(LiriSessionShellPlugin
    PRIVATE
        Qt6::DBus
        Liri::Session
)

qt6_finalize_target(LiriSessionShellPlugin)

install(
    TARGETS LiriSessionShellPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/liri/sessionmodules
)
