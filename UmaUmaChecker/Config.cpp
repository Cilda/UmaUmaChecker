#include "Config.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

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

			WindowX = config["WindowX"].get<int>();
			WindowY = config["WindowY"].get<int>();
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

	std::ofstream output(utility::GetExeDirectory() + L"\\config.json");
	output << std::setw(4) << config << std::endl;
}

void Config::Save()
{
	json config;

	config["WindowX"] = WindowX;
	config["WindowY"] = WindowY;

	std::ofstream output(utility::GetExeDirectory() + L"\\config.json");
	output << std::setw(4) << config << std::endl;
}

