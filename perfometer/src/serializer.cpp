// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include "serializer.h"
#include <fstream>

namespace perfometer {

serializer::serializer()
{    
}

serializer::~serializer()
{
    close();    
}

result serializer::open_file_stream(const char file_name[])
{
    m_report_file.open(file_name, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);

	if (!m_report_file)
	{
		return result::io_error;
	}

	return write_header();
}

result serializer::flush()
{
	m_report_file.flush();

	return m_report_file.fail() ? result::io_error : result::ok;
}

result serializer::close()
{
	m_report_file.close();

	return m_report_file.fail() ? result::io_error : result::ok;
}

result serializer::write_header()
{
	m_report_file << header
				  << major_version
				  << minor_version
				  << patch_version;

	if (m_report_file.fail())
	{
		return result::io_error;
	}

	write_clock_config();
	write_thread_info();

	return result::ok;
}

result serializer::write_clock_config()
{
	auto start_time = get_time();
	auto clock_frequency = get_clock_frequency();

	m_report_file << record_type::clock_configuration;

	unsigned char time_size = sizeof(clock_frequency);
	m_report_file << time_size;

	m_report_file.write(reinterpret_cast<const char*>(&clock_frequency), time_size);
	m_report_file.write(reinterpret_cast<const char*>(&start_time), time_size);

	return m_report_file.fail() ? result::io_error : result::ok;
}

result serializer::write_thread_info()
{
	m_report_file << record_type::thread_info;

	unsigned char thread_id_size = sizeof(thread_id);
	m_report_file << thread_id_size;

	thread_id id = get_thread_id();
	m_report_file.write(reinterpret_cast<const char*>(&id), thread_id_size);

	return m_report_file.fail() ? result::io_error : result::ok;
}

} // namespace perfometer
