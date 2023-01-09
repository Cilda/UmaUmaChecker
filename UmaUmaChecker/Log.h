#pragma once

#include <fstream>
#include <chrono>
#include "utility.h"

class Log final
{
public:
	Log() : writer(utility::GetExeDirectory() + L"\\output.log", std::ios::out | std::ios::app)
	{
	}

	~Log()
	{
		writer.close();
	}

	static Log* GetInstance()
	{
		static Log log;

		return &log;
	}

	template<typename... Args>
	void info(Args&... args)
	{
		writer << GetDate();
		print_log(args...);
		writer << std::endl;
		writer.flush();
	}

private:
	std::string GetDate()
	{
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		std::tm* time = std::localtime(&now);
		char buf[100];

		std::strftime(buf, sizeof(buf) / sizeof(wchar_t), "[%Y-%m-%d %H:%M:%S] ", time);
		return buf;
	}

	template<typename First, typename... Rest>
	void print_log(const First& first, const Rest&... rest)
	{
		writer << first;

		print_log(rest...);
	}

	void print_log()
	{
	}

private:
	std::ofstream writer;
};

