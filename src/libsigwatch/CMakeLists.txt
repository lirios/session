set(SOURCES
    sigwatch.cpp
    sigwatch.h
    sigwatch_p.h
)

add_library(Sigwatch STATIC ${SOURCES})
target_link_libraries(Sigwatch Qt5::Core)
target_include_directories(Sigwatch PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
