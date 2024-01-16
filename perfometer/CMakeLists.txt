# Perf-o-meter library for performance data collecting functionality

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include(cmake/gcc.cmake)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  
    include(cmake/clang.cmake)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    include(cmake/msvc.cmake)
else()
    message(FATAL_ERROR, "unsupported compiler")
endif()

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

target_include_directories (perfometer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

file(GLOB_RECURSE headers "include/perfometer/*.h")
set_target_properties(perfometer PROPERTIES PUBLIC_HEADER "${headers}")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(perfometer pthread)
endif()

set_property(TARGET perfometer PROPERTY CXX_STANDARD 11)

install(TARGETS perfometer EXPORT perfometer DESTINATION lib)
install(EXPORT perfometer FILE perfometer.cmake DESTINATION lib/cmake/perfometer)
install(FILES ${headers} DESTINATION include/perfometer)

# Perf-o-meter library usage samples
if (PERFOMETER_BUILD_SAMPLES)
    file(GLOB files "samples/*.cpp")

    foreach(file ${files})
        get_filename_component(file_name ${file} NAME_WE)
        add_executable(${file_name} ${file})
        target_link_libraries(${file_name} perfometer)
        set_property(TARGET ${file_name} PROPERTY CXX_STANDARD 11)
    endforeach()

    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(threads pthread)
    endif()
endif()

if (PERFOMETER_BUILD_TESTS)
    file(GLOB files "test/*.cpp")

    foreach(file ${files})
        get_filename_component(file_name ${file} NAME_WE)
        add_executable(${file_name} ${file})
        target_link_libraries(${file_name} perfometer)
        set_property(TARGET ${file_name} PROPERTY CXX_STANDARD 11)
        add_test(NAME ${file_name} COMMAND ${file_name})
    endforeach()
   
endif()

if (PERFOMETER_BUILD_BENCHMARKS)
    add_executable(benchmark benchmark/benchmark_register_constant_string.cpp)
    target_link_libraries(benchmark perfometer utils)
endif()
