# Perf-o-meter project for collecting and visualizing performance data

cmake_minimum_required(VERSION 3.1)

option (BUILD_PRINTER "Build printer" ON)
option (BUILD_SAMPLES "Build printer" ON)
option (BUILD_VISUALIZER "Build visualizer" ON)

project (perf-o-meter)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

#if (MSVC)
    foreach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string (TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
        set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
        set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
    endforeach (OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
#endif

add_subdirectory (perfometer)

if (BUILD_PRINTER)
    add_subdirectory (printer)
endif()

if (BUILD_SAMPLES)
    add_subdirectory (samples)
endif()

if (BUILD_VISUALIZER)
    add_subdirectory (visualizer)
endif()
