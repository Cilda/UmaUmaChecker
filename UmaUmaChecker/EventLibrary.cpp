#include <string>

#include "EventLibrary.h"
#include "utility.h"

#include <sstream>
#include <fstream>
#include <regex>

#include <wx/msgdlg.h>

#include <nlohmann/json.hpp>
#include "simstring/simstring.h"
#include "Log.h"

#include "version.h"

using json = nlohmann::ordered_json;

EventLibrary EventLib;

EventLibrary::EventLibrary()
{
	std::string BasePath = utility::to_string(utility::GetExeDirectory());
	DBPath = BasePath + "\\simstring\\";
}

EventLibrary::~EventLibrary()
{
}

void EventLibrary::Clear()
{
}

bool EventLibrary::Load()
{
	auto start = std::chrono::system_clock::now();

	CardEvent = EventData();
	CharaEvent = EventData();
	ScenarioEvent = ScenarioData();

	std::wstring path = utility::GetExeDirectory();

	DeleteDBFiles();

	if (!CardEvent.Load(path + L"\\Library\\Events.json")) wxMessageBox(wxT("Events.json の読み込みに失敗しました。"), app_name, wxICON_WARNING);
	if (!CharaEvent.Load(path + L"\\Library\\Chara.json")) wxMessageBox(wxT("Chara.json の読み込みに失敗しました。"), app_name, wxICON_WARNING);
	if (!ScenarioEvent.Load(path + L"\\Library\\ScenarioEvents.json")) wxMessageBox(wxT("ScenarioEvents.json の読み込みに失敗しました。"), app_name, wxICON_WARNING);

	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	LOG_INFO << "Loaded EventData at " << msec << " msec!";

	return true;
}

void EventLibrary::DeleteDBFiles()
{
	std::filesystem::remove_all(DBPath);
}
