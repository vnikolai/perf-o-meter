# Perf-o-meter printer to display report in readable form

add_executable(printer src/main.cpp)

target_link_libraries(printer perfometer)

set_property(TARGET printer PROPERTY CXX_STANDARD 17)
