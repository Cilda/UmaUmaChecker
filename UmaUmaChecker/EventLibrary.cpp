#include <string>

#include "EventLibrary.h"
#include "utility.h"

#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "simstring/simstring.h"

using json = nlohmann::json;

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
	EventMap.clear();
	CharaMap.clear();
	CharaEventMap.clear();
	OptionMap.clear();
	ScenarioEventMap.clear();
	Events.clear();
	Charas.clear();
	ScenarioEvents.clear();
}

bool EventLibrary::Load()
{
	LoadEvent();
	LoadChara();
	LoadScenarioEvent();
	return true;
}

bool EventLibrary::LoadEvent()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Events.json");
	if (stream.good()) {
		std::stringstream text;

		text << stream.rdbuf();

		try {
			json skills = json::parse(text.str());

			const char* types[] = { "SSR", "SR", "R" };
			for (int i = 0; i < 3; i++) {
				auto cards = skills["Skills"][types[i]];

				for (auto& card : cards.items()) {
					auto name = card.key();
					auto events = card.value()["Events"];
					std::shared_ptr<EventRoot> skill(new EventRoot());

					skill->Name = utility::ConvertUtf8ToUtf16(name.c_str());

					for (auto &e : events) {
						for (auto& choise : e.items()) {
							std::shared_ptr<EventSource> event(new EventSource());

							for (auto& option : choise.value()) {
								std::shared_ptr<EventOption> choise(new EventOption());

								choise->Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
								choise->Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
								event->Options.push_back(choise);

								if (OptionMap.find(choise->Title) == OptionMap.end()) {
									OptionMap[choise->Title] = event;
								}
							}
							
							std::wstring EventName = utility::ConvertUtf8ToUtf16(choise.key().c_str());
							event->Name = EventName;
							skill->Events[EventName] = event;

							if (EventMap.find(EventName) == EventMap.end()) {
								EventMap[EventName] = event;
							}
						}
					}

					Events.push_back(skill);
				}
			}
		}
		catch (json::exception& ex) {
			return false;
		}

		return true;
	}
	
	return false;
}

bool EventLibrary::LoadChara()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Chara.json");
	if (stream.good()) {
		std::stringstream text;

		text << stream.rdbuf();

		try {
			json charas = json::parse(text.str());

			const char* types[] = { "3", "2", "1" };
			for (int i = 0; i < 3; i++) {
				auto cards = charas["Chara"][types[i]];

				for (auto& card : cards.items()) {
					auto name = card.key();
					auto events = card.value()["Events"];
					std::shared_ptr<EventRoot> skill(new EventRoot());

					skill->Name = utility::ConvertUtf8ToUtf16(name.c_str());

					for (auto& e : events) {
						for (auto& choise : e.items()) {
							std::shared_ptr<EventSource> event(new EventSource());

							for (auto& option : choise.value()) {
								std::shared_ptr<EventOption> choise(new EventOption());

								choise->Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
								choise->Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
								event->Options.push_back(choise);
							}

							std::wstring EventName = utility::ConvertUtf8ToUtf16(choise.key().c_str());
							event->Name = EventName;
							skill->Events[EventName] = event;

							if (CharaEventMap.find(EventName) == CharaEventMap.end()) {
								CharaEventMap[EventName] = event;
							}
						}
					}

					Charas.push_back(skill);

					auto& bb = Charas.back();
					CharaMap[skill->Name] = skill;
				}
			}
		}
		catch (json::exception& ex) {
			return false;
		}

		return true;
	}

	return false;
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

						event_option->Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
						event_option->Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
						
						source->Options.push_back(event_option);
					}

					std::wstring EventName = utility::ConvertUtf8ToUtf16(event.key().c_str());
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

void EventLibrary::InitEventDB()
{
	CreateDirectoryA(DBPath.c_str(), NULL);
	CreateDirectoryA((DBPath + "event\\").c_str(), NULL);

	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "event\\events.db");

	simstring::ngram_generator gen2(3, false);
	simstring::writer_base<std::wstring> dbw2(gen2, DBPath + "event\\choises.db");

	for (auto& source : EventMap) {
		dbw.insert(source.first);

		for (auto& event : source.second->Options) {
			dbw2.insert(event->Title);
		}
	}
	dbw.close();
	dbw2.close();
}

void EventLibrary::InitCharaDB()
{
	CreateDirectoryA(DBPath.c_str(), NULL);
	CreateDirectoryA((DBPath + "chara\\").c_str(), NULL);

	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "chara\\chara.db");

	for (auto& pair : CharaEventMap) {
		dbw.insert(pair.first);
	}
	dbw.close();
}

void EventLibrary::InitScenarioEventDB()
{
	CreateDirectoryA(DBPath.c_str(), NULL);
	CreateDirectoryA((DBPath + "event\\").c_str(), NULL);

	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "event\\scenario.db");

	for (auto& scenario : ScenarioEvents) {
		for (auto& event : scenario->Events) {
			dbw.insert(event.first); // イベント名
		}
	}
	dbw.close();
}

std::shared_ptr<EventSource> EventLibrary::RetrieveEvent(const std::wstring& name)
{
	simstring::reader dbr;
	
	dbr.open(DBPath + "event\\events.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; (float)ratio >= 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	const auto& event = EventMap.find(xstrs.front());
	if (event == EventMap.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> EventLibrary::RetrieveEventFromOptionTitle(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "event\\choises.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; (float)ratio >= 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	const auto& event = OptionMap.find(xstrs.front());
	if (event == OptionMap.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> EventLibrary::RetrieveCharaEvent(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "chara\\chara.db");

	std::vector<std::wstring> xstrs;
	
	for (double ratio = 1.0; (float)ratio > 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	const auto& event = CharaEventMap.find(xstrs.front());
	if (event == CharaEventMap.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> EventLibrary::RetrieveScenarioEvent(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "event\\scenario.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; (float)ratio > 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	const auto& event = ScenarioEventMap.find(xstrs.front());
	if (event == ScenarioEventMap.end()) return nullptr;

	return event->second;
}

EventRoot* EventLibrary::GetCharacter(const std::wstring& name)
{
	const auto& itr = CharaMap.find(name);
	if (itr != CharaMap.end()) {
		return itr->second.get();
	}

	return nullptr;
}
