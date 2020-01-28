// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/config.h>
#include <cstdint>
#include <iostream>

namespace perfometer
{
    const char header[] = "PERFOMETER.";    // PERFOMETER.VER - VER is version composed from
                                            // major minor and patch versions one byte each

	enum record_type : uint8_t
	{
		clock_configuration = 1,    // 8 bit record type
                                    // 8 bit time size
                                    // time size clock frequency value
                                    // time size initial time

        thread_info = 2,            // 8 bit record type
                                    // 8 bit thread id size
                                    // thread id size initialization thread id

        thread_name = 3             // 8 bit record type
                                    // thread id size thread id
                                    // 8 bit string length
                                    // string length size string data
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
