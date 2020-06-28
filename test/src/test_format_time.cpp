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

int main(int argc, const char** argv)
{
	std::cout << 3600 << " " << visualizer::format_time(3600) << std::endl;
	std::cout << 5400 << " " << visualizer::format_time(5400) << std::endl;
	std::cout << 7200 << " " << visualizer::format_time(7200) << std::endl;

	std::cout << 10000 << " " << visualizer::format_time(10000) << std::endl;
	std::cout << 1000 << " " << visualizer::format_time(1000) << std::endl;
	std::cout << 100 << " " << visualizer::format_time(100) << std::endl;
	std::cout << 10 << " " << visualizer::format_time(10) << std::endl;
	std::cout << 1 << " " << visualizer::format_time(1) << std::endl;
	std::cout << 0.1 << " " << visualizer::format_time(0.1) << std::endl;
	std::cout << 0.5 << " " << visualizer::format_time(0.5) << std::endl;
	std::cout << 0.01 << " " << visualizer::format_time(0.01) << std::endl;
	std::cout << 0.001 << " " << visualizer::format_time(0.001) << std::endl;
	std::cout << 0.0001 << " " << visualizer::format_time(0.0001) << std::endl;
	std::cout << 0.00001 << " " << visualizer::format_time(0.00001) << std::endl;
	std::cout << 0.000001 << " " << visualizer::format_time(0.000001) << std::endl;
	std::cout << 0.0000001 << " " << visualizer::format_time(0.0000001) << std::endl;
	std::cout << 0.00000001 << " " << visualizer::format_time(0.00000001) << std::endl;
	std::cout << 0.000000001 << " " << visualizer::format_time(0.000000001) << std::endl;
	std::cout << 0.0000000001 << " " << visualizer::format_time(0.0000000001) << std::endl;
	std::cout << 7200 << " " << visualizer::format_time(7200) << std::endl;

	std::cout << 1.2 << " " << visualizer::format_time(1.2) << std::endl;
	std::cout << 1.4 << " " << visualizer::format_time(1.4) << std::endl;
	std::cout << 1.5 << " " << visualizer::format_time(1.5) << std::endl;
	std::cout << 11.5 << " " << visualizer::format_time(11.5) << std::endl;
	std::cout << 60 << " " << visualizer::format_time(60) << std::endl;
	std::cout << 72 << " " << visualizer::format_time(72) << std::endl;
	std::cout << 3659 << " " << visualizer::format_time(3659) << std::endl;
	std::cout << 3660 << " " << visualizer::format_time(3660) << std::endl;
	std::cout << 3661 << " " << visualizer::format_time(3661) << std::endl;
	std::cout << 3601 << " " << visualizer::format_time(3601) << std::endl;

	return 0;
}