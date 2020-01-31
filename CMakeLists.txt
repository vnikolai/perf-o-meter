# Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
# Perf-o-meter project for collecting and visualizing performance data

project (perf-o-meter)

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
add_subdirectory (printer)
add_subdirectory (samples)
add_subdirectory (visualizer)
