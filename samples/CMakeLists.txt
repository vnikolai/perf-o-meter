# Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
# perf-o-meter library samples

add_executable(simple src/simple.cpp)
target_link_libraries(simple perfometer)

add_executable(pause src/pause.cpp)
target_link_libraries(pause perfometer)
