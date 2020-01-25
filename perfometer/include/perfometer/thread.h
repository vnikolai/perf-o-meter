// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <thread>

namespace perfometer
{
    using thread_id = std::thread::id;
    
    inline thread_id get_thread_id()
    {
        return std::this_thread::get_id();
    }

} // namespace perfometer
