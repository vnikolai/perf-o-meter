# Perf-o-meter library usage samples

add_executable(test_utils src/test_format_time.cpp)
target_link_libraries(test_utils utils)
set_property(TARGET simple PROPERTY CXX_STANDARD 11)
