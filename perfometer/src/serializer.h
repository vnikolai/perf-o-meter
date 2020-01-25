// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/format.h>
#include <perfometer/perfometer.h>

#include <fstream>

namespace perfometer
{
    class serializer
    {
    public:
        serializer();
        ~serializer();
        
        result open_file_stream(const char fileName[]);
        result flush();
        result close();

    private:
        result write_header();
        result write_clock_config();
        result write_thread_info();

    private:
        std::ofstream m_report_file;
	};

} // namespace perfometer
