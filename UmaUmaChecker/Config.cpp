#include "Config.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <filesystem>

#include "utility.h"

using json = nlohmann::json;

Config::Config()
{
}

Config::~Config()
{
}

bool Config::Load()
{
	std::ifstream input(utility::GetExeDirectory() + L"\\config.json");

	if (input.good()) {
		try {
			std::stringstream stream;

			stream << input.rdbuf();
			json config = json::parse(stream.str());

			WindowX = config.value("WindowX", 0);
			WindowY = config.value("WindowY", 0);
			EnableDebug = config.value("Debug", false);
			SaveMissingEvent = config.value("SaveMissingEventName", false);
			ScreenshotSavePath = utility::ConvertUtf8ToUtf16(config.value("ScreenshotSavePath", "").c_str());
		}
		catch (json::exception& ex) {
			return false;
		}
	}
	else {
		Create();
	}
}

void Config::Create()
{
	json config;

	config["WindowX"] = 0;
	config["WindowY"] = 0;
	config["Debug"] = false;
	config["SaveMissingEventName"] = false;
	config["ScreenshotSavePath"] = L"";

	std::ofstream output(utility::GetExeDirectory() + L"\\config.json");
	output << std::setw(4) << config << std::endl;
}

void Config::Save()
{
	json config;

	config["WindowX"] = WindowX;
	config["WindowY"] = WindowY;
	config["Debug"] = EnableDebug;
	config["SaveMissingEventName"] = SaveMissingEvent;
	config["ScreenshotSavePath"] = std::filesystem::path(ScreenshotSavePath.begin(), ScreenshotSavePath.end()).u8string();

	std::ofstream output(utility::GetExeDirectory() + L"\\config.json");
	output << std::setw(4) << config << std::endl;
}

