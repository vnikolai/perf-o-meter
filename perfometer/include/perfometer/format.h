// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/config.h>
#include <cstdint>
#include <iostream>

namespace perfometer
{
    const char header[] = "PERFOMETER.";    // PERFOMETER.VER - VER is version composed from
                                            // major minor and patch versions one byte each

    using string_id = uint16_t;
    constexpr string_id invalid_string_id = 0;

	enum record_type : uint8_t
	{
		clock_configuration = 1,    // 8 bit record type
                                    // 8 bit time size
                                    // time size clock frequency value
                                    // time size initial time

        thread_info = 2,            // 8 bit record type
                                    // 8 bit thread id size
                                    // thread id size initialization thread id

        string = 3,                 // 8 bit record type
                                    // 16 bit string id
                                    // 8 bit string length
                                    // string length size string data

        thread_name = 4,            // 8 bit record type
                                    // thread id size thread id
                                    // 16 bit string id

        work = 5                    // 8 bit record type
                                    // 16 bit name string id
                                    // time size time start
                                    // time size time end
                                    // thread id size thread id
	};

    inline std::ostream& operator << (std::ostream& stream, const record_type& type)
    {
        stream.write(reinterpret_cast<const char *>(&type), 1);
        return stream;
    }

	inline std::istream& operator >> (std::istream& stream, record_type& type)
	{
		stream.read(reinterpret_cast<char *>(&type), 1);
		return stream;
	}

} // namespace perfometer
