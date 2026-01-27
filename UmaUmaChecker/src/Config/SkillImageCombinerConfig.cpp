#include "SkillImageCombinerConfig.h"

#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


SkillImageCombinerConfig::SkillImageCombinerConfig()
{
}

SkillImageCombinerConfig::~SkillImageCombinerConfig()
{
}

void SkillImageCombinerConfig::Load(const std::filesystem::path& path)
{
	try {
		std::ifstream input(path);
		json config = json::parse(input, nullptr, false);
		if (config.is_discarded()) config = json(json::value_t::object);

		ScrollbarThreshold = config["ScrollbarInfo"].value("Threshold", 0.01);
		auto ScanLine = config["ScrollbarInfo"]["ScrollbarScanLine"].array();
		ScrollbarScanLineStart = ScanLine[0].get<Coord>();
		ScrollbarScanLineEnd = ScanLine[1].get<Coord>();
		ScanMinCoord = config["ScrollbarInfo"]["ScanMinCoord"].get<Coord>();
		ScrollColorMin = config["ScrollbarInfo"]["ScrollColorRange"]["min"].get<Color>();
		ScrollColorMax = config["ScrollbarInfo"]["ScrollColorRange"]["max"].get<Color>();
		BarColorMin = config["ScrollbarInfo"]["BarColorRange"]["min"].get<Color>();
		BarColorMax = config["ScrollbarInfo"]["BarColorRange"]["max"].get<Color>();
		SkillListStartCoord = config["SkillListStartPoint"].get<Coord>();
		SkillDetectThreshold = config["SkillDetectThreshold"].get<double>();
	}
	catch (json::exception& ex) {
	}
}
