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

#include <utils/report_reader.h>
#include <iostream>

using namespace perfometer::utils;

struct options
{
    time_format tfmt    = time_format::automatic;
    bool statistics     = false;
};

//-----------------------------------------------------------------------------

class stats_printer : public report_reader
{
private:
    void log(const std::string& message) override
    {
        std::cout << message << std::endl;
    }

    void log_error(const std::string& message) override
    {
        std::cerr << message << std::endl;
    }

    void handle_clock_configuration(uint8_t time_size, perfometer::utils::perf_time clock_frequency, perfometer::utils::perf_time init_time) override
    {
        std::cout << "Time size " << int(time_size) << " bytes" << std::endl;
        std::cout << "Clock frequency " << clock_frequency << std::endl;
        std::cout << "Start time " << init_time << std::endl;
    }

    void handle_thread_info(uint8_t thread_id_size, int64_t main_thread_id) override
    {
        std::cout << "Thread ID size " << int(thread_id_size) << " bytes" << std::endl;
        std::cout << "Main thread " << main_thread_id << std::endl;
    }

    void handle_string(perfometer::string_id id, const std::string& string) override
    {
        std::cout << "String " << id << ":" << string << std::endl;
    }

    void handle_thread_name(int64_t thread_id, const std::string& name) override
    {
        std::cout << "Thread " << thread_id << ":" << name << std::endl;
    }
};

//-----------------------------------------------------------------------------

class report_printer : public stats_printer
{
public:
    report_printer(options opts)
        : m_options(opts)
    {
    }

    void handle_work(perfometer::string_id string_id, perf_thread_id thread_id, double time_start, double time_end) override
    {
        std::cout << "Work " << string_id << ":" << string_by_id(string_id)
                  << " on "  << thread_id << ":" << thread_name_by_id(thread_id)
                  << " started " << time_formatter(time_start, m_options.tfmt)
                  << " duration " << time_formatter(time_end - time_start, m_options.tfmt)
                  << std::endl;
    }
    
    void handle_wait(perfometer::string_id string_id, perf_thread_id thread_id, double time_start, double time_end) override
    {
        std::cout << "Wait " << string_id << ":" << string_by_id(string_id)
                  << " on "  << thread_id << ":" << thread_name_by_id(thread_id)
                  << " started " << time_formatter(time_start, m_options.tfmt)
                  << " duration " << time_formatter(time_end - time_start, m_options.tfmt)
                  << std::endl;
    }

    void handle_event(perfometer::string_id string_id, perf_thread_id thread_id, double time) override
    {
        std::cout << "Event " << string_id << ":" << string_by_id(string_id)
                  << " on " << thread_id << ":" << thread_name_by_id(thread_id)
                  <<  " fired " << time_formatter(time, m_options.tfmt)
                  << std::endl;
    }

private:
    options m_options;
};

//-----------------------------------------------------------------------------

int process(const char* filename, options opts)
{
    if (opts.statistics)
    {
        stats_printer reader;
        reader.process(filename);
        const auto& stats = reader.stats(); 
        std::cout << "Statistics " << std::endl
                  << "Report duration " << time_formatter(stats.duration, opts.tfmt) << std::endl
                  << "Num pages " << stats.num_pages << std::endl
                  << "Num blocks " << stats.num_blocks << std::endl;

        std::cout << stats.occurences.size() << std::endl;
        
        for (auto&& stat : stats.occurences)
        {
            auto string_id = stat.first;
            auto count = stat.second;
            std::cout << string_id << " " << reader.string_by_id(string_id) << " " << stat.second << std::endl;
        }
    }
    else 
    {
        report_printer printer{opts};
        printer.process(filename);
    }

    return 0;
}

void print_help()
{
    std::cout << "Usage: printer [options] [filename]" << std::endl
              << "Perf-o-meter printer loads binary report and prints it as text to stdout" << std::endl
              << "Arguments:" << std::endl
              << "-ts: force seconds time format" << std::endl
              << "-tm: force milliseconds time format" << std::endl
              << "-tM: force microseconds time format" << std::endl;

}

int main(int argc, const char** argv)
{
    const char* filename = nullptr;
    options opts{time_format::automatic, false};

    bool parameters_parsed = true;

    for (int i = 1; i < argc; ++i)
    {
        using namespace std::string_literals;

        const char* arg = argv[i];
        if (arg[0] == '-')
        {
            if (arg == "--help"s)
            {
                print_help();
                return 0;
            }
            if (arg == "--statistics"s || arg == "-s"s)
            {
                opts.statistics = true;
            }
            else if (arg == "-ts"s)
            {
                opts.tfmt = perfometer::utils::time_format::seconds;
            }
            else if (arg == "-tm"s)
            {
                opts.tfmt = perfometer::utils::time_format::milliseconds; 
            }
            else if (arg == "-tM"s)
            {
                opts.tfmt = perfometer::utils::time_format::microseconds;
            }
            else
            {
                parameters_parsed = false;
                std::cerr << "Unknown argument " << arg << std::endl;
            }
        }
        else
        {
            filename = argv[i];
        }        
    }

    if (!parameters_parsed || !filename)
    {
        std::cerr << "Incorrect arguments, aborting "<< std::endl;
        std::cerr << "printer [options] [report_file_name]" << std::endl;
        std::cerr << "printer --help" << std::endl;
        return -1;
    }

    return process(filename, opts);
}
