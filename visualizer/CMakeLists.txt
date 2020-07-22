# Perf-o-meter report visualizer

find_package(Qt5 COMPONENTS Core Gui OpenGL REQUIRED)

file(GLOB visualizer_sources "src/*.cpp")
add_executable(visualizer ${visualizer_sources})

target_link_libraries(
    visualizer

    perfometer
    utils
    Qt5::Core
    Qt5::Gui
    Qt5::OpenGL)

set_property(TARGET visualizer PROPERTY CXX_STANDARD 17)
