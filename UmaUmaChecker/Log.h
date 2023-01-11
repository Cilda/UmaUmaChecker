#pragma once

#define BOOST_USE_WINAPI_VERSION BOOST_WINAPI_VERSION_WIN7

#include <string>

#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup.hpp>

class Log
{
public:
	enum SeverityLevel {
		eDebug,
		eInfo,
		eWarning,
		eError,
		eCritical,

		eLevelMax,
	};
	using sink_text_t = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;

public:
	Log(const std::string& filename = "output.log");
	~Log();

	static Log& GetInstance()
	{
		static Log log;
		return log;
	}

	friend std::ostream& operator<<(std::ostream& stream, Log::SeverityLevel level);

public:
	boost::log::sources::severity_logger_mt<SeverityLevel> logger;

private:
	boost::shared_ptr<boost::log::sinks::text_file_backend> text_backend;
	boost::shared_ptr<sink_text_t> sink_text;
};


#define LOG_INFO BOOST_LOG_SEV(Log::GetInstance().logger, Log::eInfo)
#define LOG_DEBUG BOOST_LOG_SEV(Log::GetInstance().logger, Log::eDebug)
#define LOG_ERROR BOOST_LOG_SEV(Log::GetInstance().logger, Log::eError)
