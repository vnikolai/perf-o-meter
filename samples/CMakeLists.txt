# Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
# perf-o-meter library samples

add_executable(simple src/simple.cpp)
target_link_libraries(simple perfometer)
set_property(TARGET simple PROPERTY CXX_STANDARD 11)

add_executable(pause src/pause.cpp)
target_link_libraries(pause perfometer)
set_property(TARGET pause PROPERTY CXX_STANDARD 11)
