#include "StatusRecognizerConfig.h"

#include <fstream>

#include <wx/stdpaths.h>
#include <wx/FileName.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

StatusRecognizerConfig::StatusRecognizerConfig()
{
}

StatusRecognizerConfig::StatusRecognizerConfig(const wxString& FileName)
{
	Open(FileName);
}

StatusRecognizerConfig::~StatusRecognizerConfig()
{
}

bool StatusRecognizerConfig::Open(const wxString& FileName)
{
	std::ifstream input(FileName.ToStdWstring());

	try {
		json config = json::parse(input, nullptr, false);

		auto StatusBounds = config["StatusBounds"];

		SpeedBounds = StatusBounds.value<Rect<double>>("Speed", Rect<double>());
		StaminaBounds = StatusBounds.value<Rect<double>>("Stamina", Rect<double>());
		PowerBounds = StatusBounds.value<Rect<double>>("Power", Rect<double>());
		GutsBounds = StatusBounds.value<Rect<double>>("Guts", Rect<double>());
		WisdomBounds = StatusBounds.value<Rect<double>>("Wisdom", Rect<double>());

		return true;
	}
	catch (json::exception& ex) {
		return false;
	}
}

StatusRecognizerConfig& StatusRecognizerConfig::GetInstance()
{
	//auto st = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
	static StatusRecognizerConfig config(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + wxT("\\Assets\\StatusRecognizer.json"));

	return config;
}
