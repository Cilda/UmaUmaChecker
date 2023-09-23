#pragma once

#include <wx/string.h>

#include "Rect.h"

class StatusRecognizerConfig
{
public:
	StatusRecognizerConfig();
	StatusRecognizerConfig(const wxString& FileName);
	~StatusRecognizerConfig();

	bool Open(const wxString& FileName);

public:
	static StatusRecognizerConfig& GetInstance();

public:
	Rect<double> SpeedBounds;
	Rect<double> StaminaBounds;
	Rect<double> PowerBounds;
	Rect<double> GutsBounds;
	Rect<double> WisdomBounds;

};

