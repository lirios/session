install(
    FILES
        liri-entertainment.directory
        liri-internet.directory
        liri-main.directory
        liri-system.directory
        liri-work.directory
    DESTINATION
        "${KDE_INSTALL_DATADIR}/desktop-directories"
)

install(
    FILES
        liri-applications.menu
    DESTINATION
        "${KDE_INSTALL_SYSCONFDIR}/xdg/menus"
)
