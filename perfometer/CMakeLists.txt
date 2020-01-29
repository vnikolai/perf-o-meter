# Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
# perf-o-meter library for performance data collecting functionality

add_library (perfometer
            STATIC
            src/perfometer.cpp
            src/serializer.cpp)

target_include_directories (perfometer PUBLIC include)

set_property(TARGET perfometer PROPERTY CXX_STANDARD 17)
