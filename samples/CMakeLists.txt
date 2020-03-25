# Perf-o-meter library usage samples

add_executable(simple src/simple.cpp)
target_link_libraries(simple perfometer)
set_property(TARGET simple PROPERTY CXX_STANDARD 11)

add_executable(pause src/pause.cpp)
target_link_libraries(pause perfometer)
set_property(TARGET pause PROPERTY CXX_STANDARD 11)

add_executable(threads src/threads.cpp)
target_link_libraries(threads perfometer)
set_property(TARGET threads PROPERTY CXX_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(threads pthread)
endif()
