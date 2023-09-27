/* Copyright 2020-2023 Volodymyr Nikolaichuk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#pragma once

#include <perfometer/perfometer.h>
#include <utils/time.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace perfometer
{
    namespace utils
    {
        using perf_thread_id = int64_t; // holding at least 8 bytes
        using perf_time = uint64_t;     // holding at least 8 bytes
        using perf_string_id = perfometer::string_id;

        class report_reader
        {
        public:
            struct statistics
            {
                double duration             = 0.0f;
                size_t num_blocks           = 0;
                std::vector<std::pair<perfometer::string_id, size_t>> occurences;
            };

            report_reader();
            ~report_reader();

            perfometer::result process(const char* filename);

            const statistics& stats() const { return m_statistics; }

            const std::string& string_by_id(perf_string_id id) { return m_strings[id]; }
            const std::string& thread_name_by_id(perf_thread_id id) { return m_strings[m_threads[id]]; }

        protected:
            virtual void log(const std::string& message) {}
            virtual void log_error(const std::string& message) {}
            virtual void handle_clock_configuration(char time_size, perf_time clock_frequency, perf_time init_time) {}
            virtual void handle_thread_info(char thread_id_size, perf_thread_id main_thread_id) {}
            virtual void handle_string(perfometer::string_id id, const std::string& string) {}
            virtual void handle_thread_name(perf_thread_id thread_id, const std::string& name) {}
            virtual void handle_work(perfometer::string_id string_id, perf_thread_id thread_id, double time_start, double time_end) {}
            virtual void handle_wait(perfometer::string_id string_id, perf_thread_id thread_id, double time_start, double time_end) {}
            virtual void handle_event(perfometer::string_id string_id, perf_thread_id thread_id, double time) {}

        private:
            double convert_time(perf_time time);

        private:
            perf_time m_init_time = 0;
            perf_time m_clock_frequency = 0;

            std::unordered_map<perf_string_id, std::string>         m_strings;
            std::unordered_map<perf_thread_id, perf_string_id>      m_threads;
            std::unordered_map<perf_string_id, size_t>              m_blocks_occurences;
            statistics m_statistics;
        };
    }
}