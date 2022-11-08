#include <string>

#include "EventLibrary.h"
#include "utility.h"

#include <sstream>
#include <fstream>
#include <regex>
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
	CharasByRank.clear();
	Charas.clear();
	ScenarioEvents.clear();
}

bool EventLibrary::Load()
{
	LoadEvent();
	LoadChara();
	LoadScenarioEvent();
	DeleteDBFiles();

	InitEventDB();
	InitCharaDB();
	InitCharaNameDB();
	InitScenarioEventDB();
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
				std::vector< std::shared_ptr<EventRoot>> byRank;

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

								if (OptionMap.find(choise->Title) == OptionMap.end()) {
									OptionMap[choise->Title] = event;
								}
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
					byRank.push_back(skill);

					auto& bb = Charas.back();
					CharaMap[skill->Name] = skill;
				}

				std::sort(byRank.begin(), byRank.end(), [](std::shared_ptr<EventRoot> a, std::shared_ptr<EventRoot> b) {
					std::wregex regex(L"^［(.+?)］(.+?)$");
					std::wcmatch m1, m2;

					std::regex_search(a->Name.c_str(), m1, regex);
					std::regex_search(b->Name.c_str(), m2, regex);
					
					return m1[2].str() < m2[2].str();
				});
				CharasByRank.push_back(byRank);
				byRank.clear();
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

	for (auto& source : CharaEventMap) {
		dbw.insert(source.first);
	}
	dbw.close();

	simstring::ngram_generator gen2(3, false);
	simstring::writer_base<std::wstring> dbw2(gen2, DBPath + "chara\\choises.db");

	for (auto& source : Charas) {
		for (auto& event : source->Events) {
			for (auto option : event.second->Options) {
				if (!option->Title.empty()) dbw2.insert(option->Title);
			}
		}
	}

	dbw2.close();
}

void EventLibrary::InitCharaNameDB()
{
	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "chara\\charaname.db");

	for (auto& source : Charas) {
		dbw.insert(source->Name);
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
	if (name.empty()) return nullptr;

	simstring::reader dbr;
	
	dbr.open(DBPath + "event\\events.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; ratio > 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, name);
	}

	const auto& event = EventMap.find(match);
	if (event == EventMap.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> EventLibrary::RetrieveEventFromOptionTitle(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "event\\choises.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; ratio > 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

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

std::shared_ptr<EventSource> EventLibrary::RetrieveCharaEvent(const std::wstring& name, const std::wstring& CharaName)
{
	if (name.empty()) return nullptr;
	const auto& chara = CharaMap.find(CharaName);
	if (chara == CharaMap.end()) return nullptr;

	simstring::reader dbr;

	dbr.open(DBPath + "chara\\chara.db");

	std::vector<std::wstring> xstrs;
	
	for (double ratio = 1.0; ratio > 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, name);
	}

	const auto& event = chara->second->Events.find(match);
	if (event == chara->second->Events.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> EventLibrary::RetrieveCharaEventFromOptionTitle(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "chara\\choises.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; ratio > 0.4; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

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

std::shared_ptr<EventSource> EventLibrary::RetrieveScenarioEvent(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "event\\scenario.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; ratio > 0.4; ratio -= 0.05) {
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

std::shared_ptr<EventRoot> EventLibrary::RetrieveCharaName(const std::wstring& name)
{
	simstring::reader dbr;

	dbr.open(DBPath + "chara\\charaname.db");

	std::vector<std::wstring> xstrs;

	for (double ratio = 1.0; ratio > 0.2; ratio -= 0.05) {
		dbr.retrieve(name, simstring::cosine, ratio, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	dbr.close();

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, name);
	}

	const auto& itr = CharaMap.find(match);
	if (itr != CharaMap.end()) {
		return itr->second;
	}

	return nullptr;
}

EventRoot* EventLibrary::GetCharacter(const std::wstring& name)
{
	const auto& itr = CharaMap.find(name);
	if (itr != CharaMap.end()) {
		return itr->second.get();
	}

	return nullptr;
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
