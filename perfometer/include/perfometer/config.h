// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <cstddef>

//#define PERFOMETER_LOG_RECORD_SWAP_OVERHEAD
//#define PERFOMETER_PRINT_WORKLOG_OVERHEAD

namespace perfometer
{
    constexpr unsigned char major_version = 0;
    constexpr unsigned char minor_version = 1;
    constexpr unsigned char patch_version = 0;

    constexpr size_t records_cache_size = 1024;

} // namespace perfometer
