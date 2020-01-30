# Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
# Perf-o-meter flame-graph visualizer

find_package(Qt5 COMPONENTS Core Gui OpenGL REQUIRED)

add_executable(visualizer src/main.cpp)

target_link_libraries(
    visualizer
    
    perfometer
    Qt5::Core
    Qt5::Gui
    Qt5::OpenGL)

set_property(TARGET visualizer PROPERTY CXX_STANDARD 17)
