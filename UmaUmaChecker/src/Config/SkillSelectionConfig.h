#pragma once

#include <string>

#include "Range.h"
#include "Point.h"

class SkillSelectionConfig
{
public:
	SkillSelectionConfig();
	~SkillSelectionConfig();

private:
	Range<PointF> ScrollbarScanLine;
};

