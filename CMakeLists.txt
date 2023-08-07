# Perf-o-meter project for collecting and visualizing performance data

cmake_minimum_required(VERSION 3.8)

option (PERFOMETER_BUILD_PRINTER    "Build printer"     OFF)
option (PERFOMETER_BUILD_SAMPLES    "Build samples"     OFF)
option (PERFOMETER_BUILD_VISUALIZER "Build visualizer"  OFF)
option (PERFOMETER_BUILD_TESTS      "Build tests"       OFF)
option (LEAN_AND_MEAN               "Minimum build"     OFF)

if (PERFOMETER_BUILD_TESTS)
    enable_testing()
endif()

project (perf-o-meter VERSION 2.2.2)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#if (MSVC)
    foreach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string (TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
        set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
        set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
    endforeach (OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
#endif

add_subdirectory (perfometer)
add_subdirectory (utils)

if (PERFOMETER_BUILD_PRINTER)
    add_subdirectory (printer)
endif()

if (PERFOMETER_BUILD_VISUALIZER)
    add_subdirectory (visualizer)
endif()

