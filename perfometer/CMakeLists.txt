# Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
# perf-o-meter library for performance data collecting functionality

add_library (perfometer
            STATIC
            src/perfometer.cpp)

target_include_directories (perfometer PUBLIC include)
