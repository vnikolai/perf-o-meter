# Perf-o-meter library for performance data collecting functionality

add_library (perfometer
            STATIC
            src/perfometer.cpp
            src/serializer.cpp)

target_include_directories (perfometer PUBLIC include)

file(GLOB_RECURSE headers "include/perfometer/*.h")
set_target_properties(perfometer PROPERTIES PUBLIC_HEADER "${headers}")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(perfometer pthread)
endif()

set_property(TARGET perfometer PROPERTY CXX_STANDARD 11)

install(TARGETS perfometer
        ARCHIVE
            DESTINATION lib
        PUBLIC_HEADER
            DESTINATION include/perfometer)
