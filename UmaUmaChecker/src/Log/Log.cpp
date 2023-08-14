#include "Log.h"

#include <filesystem>
#include <codecvt>
#include <wx/string.h>
#include <boost/locale/generator.hpp>

#include "Utils/utility.h"


BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", Log::SeverityLevel)


std::unique_ptr<Log> Log::instance;


Log::Log(const std::string& filename)
{
	boost::log::add_common_attributes();
	auto core = boost::log::core::get();

	std::filesystem::path path = utility::GetExeDirectory();
	path /= filename;

	text_backend = boost::make_shared<boost::log::sinks::text_file_backend>(
		boost::log::keywords::file_name = path.string().c_str(),
		boost::log::keywords::open_mode = (std::ios::out | std::ios::app)
	);
	text_backend->auto_flush(true);

	sink_text = boost::shared_ptr<sink_text_t>(new sink_text_t(text_backend));
	sink_text->set_formatter(
		boost::log::expressions::format("[%1%][%2%]: %3%")
		% boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
		% boost::log::expressions::attr<SeverityLevel>("Severity")
		% boost::log::expressions::message
	);

	sink_text->imbue(boost::locale::generator()("ja_JP.UTF-8"));
	core->add_sink(sink_text);
}

Log::~Log()
{
}

std::ostream& operator<<(std::ostream& stream, Log::SeverityLevel level)
{
	static const char* levels[] = {
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"EXCEPTION",
		"CRITICAL"
	};

	if (level >= 0 && level < sizeof(levels) / sizeof(levels[0])) stream << levels[level];

	return stream;
}
