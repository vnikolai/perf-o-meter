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

#include <utils/time.h>
#include <iostream>

#define CHECK(a, b) if (a != b) { std::cout << "check failed " << #a << " != " << #b << std::endl; result = -1; }

int main(int argc, const char** argv)
{
	int result = 0;

	CHECK(visualizer::format_time(3600), "1 h");
	CHECK(visualizer::format_time(5400), "1 h 30 m");
	CHECK(visualizer::format_time(7200), "2 h");

	CHECK(visualizer::format_time(10000), "2 h 46 m 40 s");
	CHECK(visualizer::format_time(1000), "16 m 40 s");
	CHECK(visualizer::format_time(100), "1 m 40 s");
	CHECK(visualizer::format_time(10), "10 s");
	CHECK(visualizer::format_time(1), "1 s");
	CHECK(visualizer::format_time(0.1), "100 ms");
	CHECK(visualizer::format_time(0.5), "500 ms");
	CHECK(visualizer::format_time(0.01), "10 ms");
	CHECK(visualizer::format_time(0.001), "1 ms");
	CHECK(visualizer::format_time(0.0001), "100 us");
	CHECK(visualizer::format_time(0.00001), "10 us");
	CHECK(visualizer::format_time(0.000001), "1 us");
	CHECK(visualizer::format_time(0.0000001), "100 ns");
	CHECK(visualizer::format_time(0.00000001), "10 ns");
	CHECK(visualizer::format_time(0.000000001), "1 ns");
	CHECK(visualizer::format_time(0.0000000001), "0 ns");

	CHECK(visualizer::format_time(1.2), "1.20 s");
	CHECK(visualizer::format_time(1.4), "1.40 s");
	CHECK(visualizer::format_time(1.5), "1.50 s");
	CHECK(visualizer::format_time(11.5), "11.5 s");
	CHECK(visualizer::format_time(60), "1 m");
	CHECK(visualizer::format_time(72), "1 m 12 s");
	CHECK(visualizer::format_time(3659), "1 h 59 s");
	CHECK(visualizer::format_time(3660), "1 h 1 m");
	CHECK(visualizer::format_time(3661), "1 h 1 m 1 s");
	CHECK(visualizer::format_time(3601), "1 h 1 s");

	return result;
}