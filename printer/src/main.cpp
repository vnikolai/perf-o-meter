// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/format.h>
#include <algorithm>
#include <fstream>
#include <cstddef>

using thread_id = int64_t;
using time = uint64_t;

class reader : public std::ifstream
{
public:
	reader(const char* file_name)
		: std::ifstream(file_name)
	{
	}

	reader& operator >> (unsigned char& byte)
	{
		read(reinterpret_cast<char*>(&byte), 1);
		return *this;
	}

	reader& operator >> (thread_id& id)
	{
		id = 0; // potentially clearing upper part if size < 8

		read(reinterpret_cast<char*>(&id), m_thread_id_size);

		return *this;
	}

	reader& operator >> (time& time)
	{
		time = 0; // potentially clearing upper part if size < 8

		read(reinterpret_cast<char*>(&time), m_time_size);

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

int main(int argc, const char** argv)
{
	if (argc < 2)
	{
		std::cout << "Not enough arguments. Use printer [report_file_name]" << std::endl;
		return -1;
	}

	reader report_file(argv[1]);

	if (!report_file)
	{
		std::cout << "Cannot open file " << argv[1] << std::endl;
		return -1;
	}

	std::cout << "Opening report file " << argv[1] << std::endl;

	char header[16];
	report_file.read(header, 11);
	header[11] = 0;

	if (report_file.fail() || strcmp(header, perfometer::header))
	{
		std::cout << "Wrong file format " << argv[1] << std::endl;
		return -1;
	}

	unsigned char major_version = 0;
	unsigned char minor_version = 0;
	unsigned char patch_version = 0;

	report_file >> major_version
				>> minor_version
				>> patch_version;

	std::snprintf(header, 16, "%d.%d.%d", int(major_version), int(minor_version), int(patch_version));
	std::cout << "File version " << header << std::endl;

	constexpr size_t buffer_size = 1024;
	char buffer[buffer_size];

	time clock_frequency = 0;
	time start_time = 0;

	thread_id main_thread_id = 0;

	perfometer::record_type record;

	while ((report_file >> record) && !report_file.eof())
	{
		if (report_file.fail())
		{
			std::cout << "Error reading file " << argv[1] << std::endl;
			return -1;
		}

		switch (record)
		{
			case perfometer::record_type::clock_configuration:
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
							>> start_time;

				std::cout << "Time size " << int(time_size) << " bytes" << std::endl;
				std::cout << "Clock frequency " << clock_frequency << std::endl;
				std::cout << "Start time " << start_time << std::endl;
				
				break;
			}
			case perfometer::record_type::thread_info:
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
			case perfometer::record_type::thread_name:
			{
				thread_id id = 0;
				report_file >> id;

				report_file.read_string(buffer, buffer_size);

				std::cout << "Thread ID " << id << " name " << buffer << std::endl;

				break;
			}
			default:
			{
				std::cout << "ERROR: Unknown record type " << record << std::endl;
				return -1;
			}
		}
	}

	return 0;
}
