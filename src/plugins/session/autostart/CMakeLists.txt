if(NOT TARGET Liri::Xdg)
    find_package(Liri1Xdg REQUIRED)
endif()

qt6_add_plugin(LiriSessionAutostartPlugin
    STATIC
    CLASS_NAME AutostartPlugin
    MANUAL_FINALIZATION
    plugin.cpp plugin.h
)

#set_target_properties(LiriSessionAutostartPlugin PROPERTIES OUTPUT_NAME autostart)

target_link_libraries(LiriSessionAutostartPlugin
    PRIVATE
        Qt6::DBus
        Liri::Session
        Liri::Xdg
)

qt6_finalize_target(LiriSessionAutostartPlugin)

install(
    TARGETS LiriSessionAutostartPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/liri/sessionmodules
)
