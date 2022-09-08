#include "Config.h"

#include <fstream>
#include <nlohmann/json.hpp>

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
	return false;
}

void Config::Create()
{
	json config;

	config["WindowX"] = WindowX;
	config["WindowY"] = WindowY;

	std::ofstream output(utility::GetExeDirectory() + L"\\config.json");
	output << std::setw(4) << config << std::endl;
}
