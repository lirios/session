liri_add_qml_plugin(session
    MODULE_PATH
        "Liri/Session"
    VERSION
        "1.0"
    SOURCES
        plugin.cpp
        qmllauncher.cpp qmllauncher.h
        qmlsessionmanager.cpp qmlsessionmanager.h
    QML_FILES
        qmldir
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt5::Core
        Qt5::DBus
)
