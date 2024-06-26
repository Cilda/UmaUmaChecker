#pragma once

#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup.hpp>

#include <string>

class Log
{
public:
	enum SeverityLevel {
		eDebug,
		eInfo,
		eWarning,
		eError,
		eException,
		eCritical,
	};
	using sink_text_t = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;

public:
	Log(const std::string& filename = "output.log");
	~Log();

	static void Create()
	{
		if (!instance) instance = std::unique_ptr<Log>(new Log());
	}

	static Log* GetInstance()
	{
		return instance.get();
	}

	friend std::ostream& operator<<(std::ostream& stream, Log::SeverityLevel level);

public:
	boost::log::sources::wseverity_logger_mt<SeverityLevel> logger;

private:
	boost::shared_ptr<boost::log::sinks::text_file_backend> text_backend;
	boost::shared_ptr<sink_text_t> sink_text;

	static std::unique_ptr<Log> instance;
};


#define LOG_INFO BOOST_LOG_SEV(Log::GetInstance()->logger, Log::eInfo)
#define LOG_DEBUG BOOST_LOG_SEV(Log::GetInstance()->logger, Log::eDebug)
#define LOG_WARNING BOOST_LOG_SEV(Log::GetInstance()->logger, Log::eWarning)
#define LOG_ERROR BOOST_LOG_SEV(Log::GetInstance()->logger, Log::eError)
#define LOG_EXCEPTION BOOST_LOG_SEV(Log::GetInstance()->logger, Log::eException)