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

#include "serializer.h"
#include <fstream>
#include <algorithm>
#include <cstring>

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

	return status();
}

result serializer::close()
{
	m_report_file.close();

	return status();
}

serializer& serializer::operator << (const unsigned char byte)
{
	m_report_file.write(reinterpret_cast<const char*>(&byte), 1);
	return *this;
}

serializer& serializer::operator << (const char* string)
{
	unsigned char string_length = std::min(255, static_cast<int>(std::strlen(string)));
	*this << string_length;

	m_report_file.write(string, string_length);

	return *this;
}

serializer& serializer::operator << (const string_id& value)
{
	m_report_file.write(reinterpret_cast<const char*>(&value), sizeof(string_id));
	return *this;
}

serializer& serializer::operator << (const record_type type)
{
	m_report_file << type;
	return *this;
}

serializer& serializer::operator << (const thread_id& id)
{
	m_report_file.write(reinterpret_cast<const char *>(&id), sizeof(thread_id));
	return *this;
}

serializer& serializer::operator << (const time& time)
{
	m_report_file.write(reinterpret_cast<const char*>(&time), sizeof(time));
	return *this;
}

result serializer::write_header()
{
	m_report_file << header;

	*this << major_version
		  << minor_version
		  << patch_version;

	if (m_report_file.fail())
	{
		return result::io_error;
	}

	write_clock_config();
	write_thread_info();

	return status();
}

result serializer::write_clock_config()
{
	unsigned char time_size = sizeof(time);
	auto start_time = get_time();
	auto clock_frequency = get_clock_frequency();	

	*this << record_type::clock_configuration
		  << time_size
		  << clock_frequency
		  << start_time;

	return status();
}

result serializer::write_thread_info()
{
	unsigned char thread_id_size = sizeof(thread_id);
	thread_id id = get_thread_id();

	*this << record_type::thread_info	
		  << thread_id_size
		  << id;

	return status();
}

} // namespace perfometer
