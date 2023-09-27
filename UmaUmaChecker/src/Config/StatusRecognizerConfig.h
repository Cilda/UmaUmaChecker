#pragma once

#include <wx/string.h>

#include "Rect.h"
#include "RGB.h"

class StatusRecognizerConfig
{
private:
	StatusRecognizerConfig();
	StatusRecognizerConfig(const wxString& FileName);
	~StatusRecognizerConfig();

	bool Open(const wxString& FileName);

public:
	static bool Load();
	static StatusRecognizerConfig& Get();

public:
	Rect<double> SpeedBounds;
	Rect<double> StaminaBounds;
	Rect<double> PowerBounds;
	Rect<double> GutsBounds;
	Rect<double> WisdomBounds;
	RGB DetectColor;
	float Threshold;

private:
	static StatusRecognizerConfig instance;
};

