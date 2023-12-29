qt6_add_plugin(LiriSessionServicesPlugin
    STATIC
    CLASS_NAME ServicesPlugin
    MANUAL_FINALIZATION
    plugin.cpp plugin.h
)

#set_target_properties(LiriSessionServicesPlugin PROPERTIES OUTPUT_NAME services)

target_link_libraries(LiriSessionServicesPlugin
    PRIVATE
        Qt6::DBus
        Liri::Session
)

qt6_finalize_target(LiriSessionServicesPlugin)

install(
    TARGETS LiriSessionServicesPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/liri/sessionmodules
)

