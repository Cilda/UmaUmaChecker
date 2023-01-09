#include <string>

#include "EventLibrary.h"
#include "utility.h"

#include <sstream>
#include <fstream>
#include <regex>

#include <wx/msgdlg.h>

#include <nlohmann/json.hpp>
#include "simstring/simstring.h"

#include "version.h"

using json = nlohmann::json;

const double EventLibrary::MIN_DEFAULT_RATIO = 0.4;
const double EventLibrary::MIN_CHARA_RATIO = 0.4;

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
	OptionMap.clear();
	ScenarioEventMap.clear();
	ScenarioEvents.clear();
}

bool EventLibrary::Load()
{
	CardEvent = EventData();
	CharaEvent = EventData();
	std::wstring path = utility::GetExeDirectory();

	DeleteDBFiles();

	if (!CardEvent.Load(path + L"\\Library\\Events.json")) wxMessageBox(wxT("Events.json の読み込みに失敗しました。"), app_name, wxICON_WARNING);
	if (!CharaEvent.Load(path + L"\\Library\\Chara.json")) wxMessageBox(wxT("Chara.json の読み込みに失敗しました。"), app_name, wxICON_WARNING);
	if (!LoadScenarioEvent()) wxMessageBox(wxT("ScenarioEvents.json の読み込みに失敗しました。"), app_name, wxICON_WARNING);

	InitScenarioEventDB();
	return true;
}

bool EventLibrary::LoadScenarioEvent()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\ScenarioEvents.json");
	if (stream.good()) {
		std::stringstream text;

		text << stream.rdbuf();

		try {
			json events = json::parse(text.str());

			const std::filesystem::path types[] = { L"URA", L"アオハル", L"クラマ", L"グラライ" };
			for (int i = 0; i < 4; i++) {
				auto scenario = events[types[i].u8string()];
				std::shared_ptr<EventRoot> root(new EventRoot());

				for (auto& event : scenario.items()) {
					std::shared_ptr<EventSource> source(new EventSource());
					
					for (auto& option : event.value()) {
						std::shared_ptr<EventOption> event_option(new EventOption());

						event_option->Title = utility::from_u8string(option["Title"].get<std::string>());
						event_option->Effect = utility::from_u8string(option["Effect"].get<std::string>());
						
						source->Options.push_back(event_option);

						if (OptionMap.find(event_option->Title) == OptionMap.end()) {
							OptionMap[event_option->Title] = source;
						}
					}

					std::wstring EventName = utility::from_u8string(event.key());
					source->Name = EventName;
					root->Events[EventName] = source;
					ScenarioEventMap[EventName] = source;
				}

				root->Name = types[i].wstring();
				ScenarioEvents.push_back(root);
			}
		}
		catch (json::exception& ex) {
			return false;
		}

		return true;
	}

	return false;
}

void EventLibrary::InitScenarioEventDB()
{
	CreateDirectoryA(DBPath.c_str(), NULL);
	CreateDirectoryA((DBPath + "event\\").c_str(), NULL);

	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "event\\scenario.db");

	simstring::ngram_generator gen2(3, false);
	simstring::writer_base<std::wstring> dbw2(gen2, DBPath + "event\\scenario_choises.db");

	for (auto& scenario : ScenarioEvents) {
		for (auto& event : scenario->Events) {
			dbw.insert(event.first); // イベント名
			for (auto& option : event.second->Options) {
				dbw2.insert(option->Title);
			}
		}
	}
	dbw.close();
	dbw2.close();
}

std::shared_ptr<EventSource> EventLibrary::RetrieveScenarioEvent(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "event\\scenario.db");

	std::vector<std::wstring> xstrs;

	Retrieve(dbr, MIN_DEFAULT_RATIO, name, xstrs);

	dbr.close();

	if (xstrs.empty()) return nullptr;

	const auto& event = ScenarioEventMap.find(xstrs.front());
	if (event == ScenarioEventMap.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> EventLibrary::RetrieveScenarioEventFromOptionTitle(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "event\\scenario_choises.db");

	std::vector<std::wstring> xstrs;

	Retrieve(dbr, MIN_DEFAULT_RATIO, name, xstrs);

	dbr.close();

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, name);
	}

	const auto& event = OptionMap.find(match);
	if (event == OptionMap.end()) return nullptr;

	return event->second;
}

EventRoot* EventLibrary::GetCharacter(const std::wstring& name)
{
	auto root = CharaEvent.GetName(name);
	return root.get();
}

void EventLibrary::DeleteDBFiles()
{
	std::filesystem::remove_all(DBPath);
}

std::wstring EventLibrary::GetBestMatchString(const std::vector<std::wstring>& xstrs, const std::wstring& text)
{
	simstring::ngram_generator gen(1, false);

	double max_rate = 0.0;
	std::wstring best_match;
	std::vector<std::wstring> basengrams;

	gen(text, std::inserter(basengrams, basengrams.end()));

	for (auto& str : xstrs) {
		std::vector<std::wstring> ngrams;
		int total = 0, equal = 0;

		gen(str, std::inserter(ngrams, ngrams.end()));

		for (auto& word1 : basengrams) {
			for (auto& word2 : ngrams) {
				if (word1 == word2) {
					equal++;
				}
			}

			total++;
		}

		if (max_rate < (double)equal / total) {
			max_rate = (double)equal / total;
			best_match = str;
		}
	}

	return best_match;
}

void EventLibrary::Retrieve(simstring::reader& dbr, double MinRatio, const std::wstring& name, std::vector<std::wstring>& xstrs)
{
	for (double ratio = 1.0; ratio > MinRatio; ratio -= 0.1) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}
}
