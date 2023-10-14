#include <string>

#include "EventLibrary.h"
#include "Utils/utility.h"

#include <sstream>
#include <fstream>
#include <regex>

#include <wx/msgdlg.h>

#include <nlohmann/json.hpp>
#include "simstring/simstring.h"
#include "Log/Log.h"

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

	if (!CardEvent.Load(path + L"\\Library\\Events.json")) {
		wxMessageBox(wxT("サポートカードイベントの読み込みに失敗しました。"), app_name, wxICON_ERROR);
		return false;
	}
	if (!CharaEvent.Load(path + L"\\Library\\Chara.json")) {
		wxMessageBox(wxT("キャライベントの読み込みに失敗しました。"), app_name, wxICON_ERROR);
		return false;
	}
	if (!ScenarioEvent.Load(path + L"\\Library\\ScenarioEvents.json")) {
		wxMessageBox(wxT("シナリオイベントの読み込みに失敗しました。"), app_name, wxICON_ERROR);
		return false;
	}
	if (!LoadSkills()) {
		wxMessageBox(wxT("スキルデータの読み込みに失敗しました。"), app_name, wxICON_ERROR);
		return false;
	}

	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	LOG_INFO << "Loaded EventData at " << msec << " msec!";

	return true;
}

void EventLibrary::DeleteDBFiles()
{
	std::filesystem::remove_all(DBPath);
}

bool EventLibrary::LoadSkills()
{
	SkillMap.clear();

	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Skills.json");
	if (stream.good()) {
		try {
			json skills = json::parse(stream);

			for (auto skill : skills) {
				SkillMap[utility::from_u8string(skill["Name"].get<std::string>())] = utility::from_u8string(skill["Description"].get<std::string>());
			}
		}
		catch (json::exception& ex) {
			return false;
		}

		return true;
	}

	return false;
}
