// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <chrono>

namespace perfometer
{
    using clock = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using time = clock::duration::rep;

    inline time get_clock_frequency()
    {
        return clock::period::den;
    }

    inline time get_time()
    {
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }

} // namespace perfometer
