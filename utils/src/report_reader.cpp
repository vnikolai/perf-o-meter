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
#include <perfometer/format.h>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

#define LOG(...)        { std::stringstream s; s << __VA_ARGS__; log(s.str().c_str()); }
#define LOG_ERROR(...)  { std::stringstream s; s << __VA_ARGS__; log_error(s.str().c_str()); }

namespace perfometer {
namespace utils {

template<typename stream>
class binary_stream_reader : public stream
{
public:
    template<typename... Args>
    binary_stream_reader(Args&&... args)
        : stream(std::forward<Args>(args)...)
    {
    }

    binary_stream_reader& operator >> (unsigned char& byte)
    {
        stream::read(reinterpret_cast<char*>(&byte), 1);
        return *this;
    }

    binary_stream_reader& operator >> (perf_thread_id& id)
    {
        id = 0; // potentially clearing upper part if size < 8

        stream::read(reinterpret_cast<char*>(&id), m_thread_id_size);

        return *this;
    }

    binary_stream_reader& operator >> (perf_time& time)
    {
        time = 0; // potentially clearing upper part if size < 8

        stream::read(reinterpret_cast<char*>(&time), m_time_size);

        return *this;
    }

    binary_stream_reader& operator >> (perf_string_id& id)
    {
        stream::read(reinterpret_cast<char*>(&id), sizeof(perf_string_id));

        return *this;
    }

    binary_stream_reader& read_string(char* buffer, size_t buffer_size)
    {
        unsigned char name_length = 0;
        *this >> name_length;

        size_t length = std::min(buffer_size - 1, static_cast<size_t>(name_length));

        stream::read(buffer, length);
        buffer[length] = 0;

        return *this;
    }

    void set_thread_id_size(size_t size) { m_thread_id_size = size; }
    void set_time_size(size_t size) { m_time_size = size; }

private:
    size_t m_thread_id_size = 0;
    size_t m_time_size = 0;
};

report_reader::report_reader()
{
}

report_reader::~report_reader()
{
}

double report_reader::convert_time(perf_time time)
{
    return static_cast<double>(time - m_init_time) / m_clock_frequency;
}

perfometer::result report_reader::process(const char* filename)
{
    binary_stream_reader<std::ifstream> report_file(filename, std::ios::binary | std::ios::ate);

    if (!report_file)
    {
        LOG_ERROR( "Cannot open file " << filename );
        return perfometer::result::file_not_found;
    }

    LOG( "Opening report file " << filename );

    const size_t report_size = report_file.tellg();
    report_file.seekg(0);
    size_t progress = 0;

    char header[16];
    report_file.read(header, 11);
    header[11] = 0;

    if (report_file.fail() || std::strncmp(header, perfometer::format::header, 11))
    {
        LOG_ERROR( "Wrong file format " << filename );
        return perfometer::result::wrong_format;
    }

    unsigned char major_version = 0;
    unsigned char minor_version = 0;
    unsigned char patch_version = 0;

    report_file >> major_version
                >> minor_version
                >> patch_version;

    LOG( "File version "
         << int(major_version) << "."
         << int(minor_version) << "."
         << int(patch_version) );

    int report_version = major_version << 16 | minor_version << 8 | patch_version;
    int software_version = perfometer::format::major_version << 16 |
                           perfometer::format::minor_version << 8 |
                           perfometer::format::patch_version;

    if (report_version > software_version)
    {
        LOG_ERROR( "Report version is newer than current software supports"
                  << int(major_version) << "." << int(minor_version) << "." << int(patch_version) );
        return perfometer::result::newer_format;
    }

    constexpr size_t buffer_size = 1024;
    char buffer[buffer_size];

    perf_time duration = 0;
    perf_thread_id main_thread_id = 0;

    std::streampos page_end = -1;
    perfometer::format::record_type record_type;

    while ((report_file >> record_type) && !report_file.eof())
    {
        if (report_file.fail())
        {
            LOG_ERROR( "Error reading file " << filename )
            return perfometer::result::io_error;
        }

        size_t current_progress = report_file.tellg() * 100 / report_size;
        if (current_progress > progress)
        {
            progress = current_progress;
            handle_loading_progress(progress);
        }

        m_statistics.num_blocks++;

        switch (record_type)
        {
            case perfometer::format::record_type::clock_configuration:
            {
                unsigned char time_size = 0;
                report_file >> time_size;

                if (time_size > 8)
                {
                    LOG_ERROR( "ERROR: Time size too large" );
                    return perfometer::result::invalid_arguments;
                }

                report_file.set_time_size(time_size);

                report_file >> m_clock_frequency
                            >> m_init_time;

                handle_clock_configuration(time_size, m_clock_frequency, m_init_time);

                break;
            }
            case perfometer::format::record_type::thread_info:
            {
                unsigned char thread_id_size = 0;
                report_file >> thread_id_size;

                if (thread_id_size > 8)
                {
                    LOG_ERROR( "ERROR: Thread id size too large" );
                    return perfometer::result::invalid_arguments;
                }

                report_file.set_thread_id_size(thread_id_size);

                report_file >> main_thread_id;

                handle_thread_info(thread_id_size, main_thread_id);

                break;
            }
            case perfometer::format::record_type::page:
            {
                uint16_t page_size = 0;
                report_file >> page_size;

                page_end = report_file.tellg() + std::streampos(page_size);

                perf_thread_id page_thread_id = 0;
                report_file >> page_thread_id;

                m_statistics.num_pages++;

                LOG( "reading page " << page_size << " bytes with thread id " << page_thread_id );

                break;
            }
            case perfometer::format::record_type::page_end:
            {
                page_end = -1;

                LOG( "page ended" );

                break;
            }
            case perfometer::format::record_type::string:
            {
                perf_string_id id = 0;
                report_file >> id;

                report_file.read_string(buffer, buffer_size);

                handle_string(id, m_strings[id] = buffer);

                break;
            }
            case perfometer::format::record_type::thread_name:
            {
                perf_thread_id thread_id = 0;
                perf_string_id string_id = 0;

                report_file >> thread_id
                            >> string_id;

                m_threads[thread_id] = string_id;

                handle_thread_name(thread_id, m_strings[string_id]);

                break;
            }
            case perfometer::format::record_type::work:
            case perfometer::format::record_type::wait:
            {
                perf_string_id string_id = 0;
                perf_thread_id thread_id = 0;
                perf_time time_start = 0;
                perf_time time_end = 0;

                report_file >> string_id
                            >> time_start
                            >> time_end
                            >> thread_id;

                m_blocks_occurences.emplace(string_id, 0).first->second++;
                duration = std::max<perf_time>(duration, time_end - m_init_time);

                if (record_type == perfometer::format::record_type::work)
                {
                    handle_work(string_id, thread_id, convert_time(time_start), convert_time(time_end));
                }
                else if (record_type == perfometer::format::record_type::wait)
                {
                    handle_wait(string_id, thread_id, convert_time(time_start), convert_time(time_end));
                }

                break;
            }
            case perfometer::format::record_type::event:
            {
                perf_string_id string_id = 0;
                perf_thread_id thread_id = 0;
                perf_time t = 0;

                report_file >> string_id
                            >> t
                            >> thread_id;

                m_blocks_occurences.emplace(string_id, 0).first->second++;
                duration = std::max<perf_time>(duration, t - m_init_time);

                handle_event(string_id, thread_id, convert_time(t));

                break;
            }
            default:
            {
                LOG_ERROR( "ERROR: Unknown record type " << int(record_type) );

                if (page_end > report_file.tellg() && page_end < report_size)
                {
                    report_file.seekg(page_end);

                    LOG_ERROR( "Jumping to end of page at " << page_end );
                }
                else
                {
                    LOG_ERROR( "No next page" )
                    return perfometer::result::io_error;
                }
            }
        }
    }

    m_statistics.duration = static_cast<double>(duration) / m_clock_frequency;

    m_statistics.occurences.reserve(m_blocks_occurences.size());
    std::copy(m_blocks_occurences.begin(), m_blocks_occurences.end(), std::back_inserter(m_statistics.occurences));
    std::sort(m_statistics.occurences.begin(), m_statistics.occurences.end(), [](const auto& left, const auto& right)
    {
        return left.second < right.second;
    });

    return perfometer::result::ok;
}

} // namespace utils
} // namespace perfometer
