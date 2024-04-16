# Perf-o-meter report visualizer

find_package(Qt6 COMPONENTS Core Gui OpenGL Widgets OpenGLWidgets REQUIRED)

file(GLOB visualizer_sources "src/*.cpp")
add_executable(visualizer ${visualizer_sources})

target_link_libraries(
    visualizer

    perfometer
    utils
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Widgets
    Qt6::OpenGLWidgets)

set_property(TARGET visualizer PROPERTY CXX_STANDARD 20)
