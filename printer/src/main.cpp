/* Copyright 2020 Volodymyr Nikolaichuk

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

#include <perfometer/format.h>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <utils/time.h>

using perf_thread_id = int64_t;		// holding at least 8 bytes
using perf_time = uint64_t;			// holding at least 8 bytes
using perf_string_id = perfometer::string_id;

namespace perfometer {
class reader : public std::ifstream
{
public:
	reader(const char* file_name,
		   std::ios::openmode openMode = std::ios::binary)
		: std::ifstream(file_name, openMode)
	{
	}

	reader& operator >> (unsigned char& byte)
	{
		read(reinterpret_cast<char*>(&byte), 1);
		return *this;
	}

	reader& operator >> (perf_thread_id& id)
	{
		id = 0; // potentially clearing upper part if size < 8

		read(reinterpret_cast<char*>(&id), m_thread_id_size);

		return *this;
	}

	reader& operator >> (perf_time& time)
	{
		time = 0; // potentially clearing upper part if size < 8

		read(reinterpret_cast<char*>(&time), m_time_size);

		return *this;
	}

	reader& operator >> (perf_string_id& id)
	{
		read(reinterpret_cast<char*>(&id), sizeof(perf_string_id));

		return *this;
	}

	reader& read_string(char* buffer, size_t buffer_size)
	{
		unsigned char name_length = 0;
		*this >> name_length;

		size_t length = std::min(buffer_size - 1, static_cast<size_t>(name_length));

		read(buffer, length);
		buffer[length] = 0;

		return *this;
	}

	void set_thread_id_size(size_t size) { m_thread_id_size = size; }
	void set_time_size(size_t size) { m_time_size = size; }

private:
	size_t m_thread_id_size = 0;
	size_t m_time_size = 0;
};

//-----------------------------------------------------------------------------

int process(const char* filename, utils::time_format tfmt)
{
	reader report_file(filename);

	if (!report_file)
	{
		std::cerr << "Cannot open file " << filename << std::endl;
		return -1;
	}

	std::cout << "Opening report file " << filename << std::endl;

	char header[16];
	report_file.read(header, 11);
	header[11] = 0;

	if (report_file.fail() || std::strncmp(header, perfometer::format::header, 11))
	{
		std::cout << "Wrong file format " << filename << std::endl;
		return -1;
	}

	unsigned char major_version = 0;
	unsigned char minor_version = 0;
	unsigned char patch_version = 0;

	report_file >> major_version
				>> minor_version
				>> patch_version;

	std::cout << "File version "
			  << int(major_version) << "."
			  << int(minor_version) << "."
			  << int(patch_version) << std::endl;

	constexpr size_t buffer_size = 1024;
	char buffer[buffer_size];

	perf_time clock_frequency = 0;
	perf_time init_time = 0;

	perf_thread_id main_thread_id = 0;

	std::unordered_map<perf_string_id, std::string>		strings;
	std::unordered_map<perf_thread_id, perf_string_id>	threads;

	perfometer::format::record_type record_type;

	while ((report_file >> record_type) && !report_file.eof())
	{
		using namespace perfometer::utils;

		if (report_file.fail())
		{
			std::cout << "Error reading file " << filename << std::endl;
			return -1;
		}

		switch (record_type)
		{
			case perfometer::format::record_type::clock_configuration:
			{
				unsigned char time_size = 0;
				report_file >> time_size;

				if (time_size > 8)
				{
					std::cout << "ERROR: Time size too large" << std::endl;
					return -1;
				}

				report_file.set_time_size(time_size);

				report_file >> clock_frequency
							>> init_time;

				std::cout << "Time size " << int(time_size) << " bytes" << std::endl;
				std::cout << "Clock frequency " << clock_frequency << std::endl;
				std::cout << "Start time " << init_time << std::endl;

				break;
			}
			case perfometer::format::record_type::thread_info:
			{
				unsigned char thread_id_size = 0;
				report_file >> thread_id_size;

				if (thread_id_size > 8)
				{
					std::cout << "ERROR: Thread id size too large" << std::endl;
					return -1;
				}

				report_file.set_thread_id_size(thread_id_size);

				report_file >> main_thread_id;

				std::cout << "Thread ID size " << int(thread_id_size) << " bytes" << std::endl;
				std::cout << "Main thread " << main_thread_id << std::endl;

				break;
			}
			case perfometer::format::record_type::string:
			{
				perf_string_id id = 0;
				report_file >> id;

				report_file.read_string(buffer, buffer_size);

				strings[id] = buffer;

				std::cout << "String ID " << id << " name " << buffer << std::endl;

				break;
			}
			case perfometer::format::record_type::thread_name:
			{
				perf_thread_id thread_id = 0;
				perf_string_id string_id = 0;

				report_file >> thread_id
							>> string_id;

				threads[thread_id] = string_id;

				std::cout << "Thread ID " << thread_id << " name " << strings[string_id] << std::endl;

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

				switch (record_type)
				{
					case perfometer::format::record_type::work: std::cout << "Work"; break;
					case perfometer::format::record_type::wait: std::cout << "Wait"; break;
					default: break;
				}

				std::cout << " " << string_id << ":" << strings[string_id]
						  << " on "  << thread_id << ":" << strings[threads[thread_id]]
						  << " started " << time_formatter(time_start - init_time, clock_frequency, tfmt)
						  << " duration " << time_formatter(time_end - time_start, clock_frequency, tfmt)
						  << std::endl;

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

				std::cout << "Event " << strings[string_id]
						  << " on "  << strings[threads[thread_id]]
						  <<  " fired " << time_formatter(t - init_time, clock_frequency, tfmt)
						  << std::endl;

				break;
			}
			default:
			{
				std::cout << "ERROR: Unknown record type " << record_type << std::endl;
				return -1;
			}
		}
	}

	return 0;
}

} // namespace perfometer

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
	perfometer::utils::time_format tfmt = perfometer::utils::time_format::automatic;

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
			else if (arg == "-ts"s)
			{
				tfmt = perfometer::utils::time_format::seconds;
			}
			else if (arg == "-tm"s)
			{
				tfmt = perfometer::utils::time_format::milliseconds; 
			}
			else if (arg == "-tM"s)
			{
				tfmt = perfometer::utils::time_format::microseconds;
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

	return perfometer::process(filename, tfmt);
}
