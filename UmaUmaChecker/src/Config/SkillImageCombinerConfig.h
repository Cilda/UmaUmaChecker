#pragma once

#include "IConfig.h"
#include "Utils/Coord.h"
#include "Utils/Color.h"

class SkillImageCombinerConfig : public IConfig
{
public:
	SkillImageCombinerConfig();
	~SkillImageCombinerConfig();

	void Load(const std::filesystem::path& path) override;

public:
	double ScrollbarThreshold;

	Coord ScanMinCoord;
	Coord ScrollbarScanLineStart;
	Coord ScrollbarScanLineEnd;

	Color ScrollColorMin;
	Color ScrollColorMax;

	Color BarColorMin;
	Color BarColorMax;

	Coord SkillListStartCoord;

	double SkillDetectThreshold;
};

