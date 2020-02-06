// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <mutex>

namespace perfometer
{
    using mutex = std::recursive_mutex;
    using scoped_lock = std::unique_lock<mutex>;

} // namespace perfometer
