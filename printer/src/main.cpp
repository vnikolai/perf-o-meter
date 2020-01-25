// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/format.h>
#include <fstream>

int main(int argc, const char** argv)
{
	if (argc < 2)
	{
		std::cout << "Not enough arguments. Use printer [report_file_name]" << std::endl;
		return -1;
	}

	std::ifstream report_file(argv[1]);

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

	char buffer[1024];

	unsigned char time_size = 0;
	uint64_t clock_frequency = 0;
	uint64_t start_time = 0;

	unsigned char thread_id_size = 0;
	uint64_t main_thread_id = 0;

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
				report_file >> time_size;

				if (time_size > 8)
				{
					report_file.read(buffer, time_size);
					report_file.read(buffer, time_size);
					std::cout << "ERROR: Time size too large" << std::endl;
				}
				else
				{
					report_file.read(reinterpret_cast<char*>(&clock_frequency), time_size);
					report_file.read(reinterpret_cast<char*>(&start_time), time_size);

					std::cout << "Time size " << int(time_size) << " bytes" << std::endl;
					std::cout << "Clock frequency " << clock_frequency << std::endl;
					std::cout << "Start time " << start_time << std::endl;					
				}
				
				break;
			}
			case perfometer::record_type::thread_info:
			{
				report_file >> thread_id_size;

				if (time_size > 8)
				{
					report_file.read(buffer, thread_id_size);
					std::cout << "ERROR: Thread id size too large" << std::endl;
				}
				else
				{
					report_file.read(reinterpret_cast<char*>(&main_thread_id), thread_id_size);

					std::cout << "Thread ID size " << int(thread_id_size) << " bytes" << std::endl;
					std::cout << "Main thread " << main_thread_id << std::endl;					
				}
				
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
