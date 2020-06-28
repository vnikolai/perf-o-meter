# Perf-o-meter library for performance data collecting functionality

add_library (utils
            STATIC
            src/time.cpp)

target_include_directories (utils PUBLIC include)

file(GLOB_RECURSE headers "include/utils/*.h")
set_target_properties(perfometer PROPERTIES PUBLIC_HEADER "${headers}")

set_property(TARGET perfometer PROPERTY CXX_STANDARD 11)
