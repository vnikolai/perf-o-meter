# Perf-o-meter library for performance data collecting functionality

add_library (perfometer
            STATIC
            src/perfometer.cpp
            src/serializer.cpp)

set(PERFOMETER_TIME_H <perfometer/time.h>)
set(PERFOMETER_THREAD_H <perfometer/thread.h>)

if (OVERRIDE_TIME_H)
    set(PERFOMETER_TIME_H ${OVERRIDE_TIME_H})
endif()

if (OVERRIDE_THREAD_H)
    set(PERFOMETER_THREAD_H ${OVERRIDE_THREAD_H})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/perfometer/config.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/perfometer/config.h)

target_include_directories (perfometer PUBLIC include)

file(GLOB_RECURSE headers "include/perfometer/*.h")
set_target_properties(perfometer PROPERTIES PUBLIC_HEADER "${headers}")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(perfometer pthread)
endif()

set_property(TARGET perfometer PROPERTY CXX_STANDARD 11)

install(TARGETS perfometer
        ARCHIVE
            DESTINATION bin
        PUBLIC_HEADER
            DESTINATION include/perfometer)
