#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>

class timer
{
public:
	timer() = default;
	void start(const std::string& text_)
	{
		text = text_;
		begin = std::chrono::system_clock::now();
	}
	void stop()
	{
		auto end = std::chrono::system_clock::now();
		auto dur = end - begin;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		std::cout << std::setw(16) << text << ":" << std::setw(5) << ms << "ms" << std::endl;
	}

private:
	std::string text;
	std::chrono::system_clock::time_point begin;
};
