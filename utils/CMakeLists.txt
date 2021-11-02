# Perf-o-meter library for performance data collecting functionality

add_library (utils
            STATIC
            src/time.cpp)

target_include_directories (utils PUBLIC include)

file(GLOB_RECURSE headers "include/utils/*.h")
set_target_properties(utils PROPERTIES PUBLIC_HEADER "${headers}")

set_property(TARGET utils PROPERTY CXX_STANDARD 11)

if (BUILD_TESTS)
    add_executable(test_format_time test/test_format_time.cpp)
    target_link_libraries(test_format_time utils)

    add_test(NAME test_format_time COMMAND test_format_time)
endif()
