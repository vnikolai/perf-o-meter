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

        result serialize_thread_name(const thread_id& id, const char* name);

    private:

        serializer& operator << (const unsigned char byte);
        serializer& operator << (const char* string);
        serializer& operator << (const record_type type);
        serializer& operator << (const thread_id& id);
        serializer& operator << (const time& time);

        result write_header();
        result write_clock_config();
        result write_thread_info();

    private:
        std::ofstream m_report_file;
	};

} // namespace perfometer
