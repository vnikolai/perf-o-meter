// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/perfometer.h>

namespace perfometer
{
    // work_logger expects const string as name

    class work_logger
    {
    public:
        work_logger(const char name[])
        {
            m_start_time = get_time();
            m_name = name;
        }

        ~work_logger()
        {
            log_work(m_name, m_start_time, get_time());
        }

    private:
        const char* m_name;
        time m_start_time;
    };
}

#if defined(__PRETTY_FUNCTION__)
#   define PERFOMETER_FUNCTION     __PRETTY_FUNCTION__
#else
#   define PERFOMETER_FUNCTION     __FUNCTION__
#endif

#define PERFOMETER_LOG_FUNCTION() perfometer::work_logger func_logger(PERFOMETER_FUNCTION)
