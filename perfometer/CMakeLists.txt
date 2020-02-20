# Perf-o-meter library for performance data collecting functionality

add_library (perfometer
            STATIC
            src/perfometer.cpp
            src/serializer.cpp)

target_include_directories (perfometer PUBLIC include)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(perfometer pthread)
endif()

set_property(TARGET perfometer PROPERTY CXX_STANDARD 11)
