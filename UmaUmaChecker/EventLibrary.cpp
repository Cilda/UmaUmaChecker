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

bool EventLibrary::Load()
{
	LoadEvent();
	LoadChara();
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
					std::shared_ptr<Character> skill(new Character());

					skill->Name = utility::ConvertUtf8ToUtf16(name.c_str());

					for (auto &e : events) {
						for (auto& choise : e.items()) {
							Character::Event event;

							for (auto& option : choise.value()) {
								Character::Choise choise;

								choise.Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
								choise.Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
								event.Choises.push_back(choise);
							}
							
							std::wstring EventName = utility::ConvertUtf8ToUtf16(choise.key().c_str());
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
					std::shared_ptr<Character> skill(new Character());

					skill->Name = utility::ConvertUtf8ToUtf16(name.c_str());

					for (auto& e : events) {
						for (auto& choise : e.items()) {
							Character::Event event;

							for (auto& option : choise.value()) {
								Character::Choise choise;

								choise.Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
								choise.Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
								event.Choises.push_back(choise);
							}

							std::wstring EventName = utility::ConvertUtf8ToUtf16(choise.key().c_str());
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

void EventLibrary::InitEventDB()
{
	CreateDirectoryA(DBPath.c_str(), NULL);
	CreateDirectoryA((DBPath + "event\\").c_str(), NULL);

	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "event\\events.db");

	for (auto& pair : EventMap) {
		dbw.insert(pair.first);
	}
	dbw.close();
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

	/*
	auto itr = CharaEventMap.find(L"êVîNÇÃï¯ïâ");
	assert(itr != CharaEventMap.end());
	*/
}

std::wstring EventLibrary::SearchEvent(const std::wstring& name)
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

	return !xstrs.empty() ? xstrs.front() : L"";
}

std::wstring EventLibrary::SearchCharaEvent(const std::wstring& name)
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

	return !xstrs.empty() ? xstrs.front() : L"";
}
