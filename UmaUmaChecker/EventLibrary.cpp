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
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Skills.json");
	if (stream.good()) {
		std::stringstream text;
		simstring::ngram_generator gen(3, false);
		simstring::writer_base<std::wstring> dbw(gen, "");

		text << stream.rdbuf();

		try {
			json skills = json::parse(text.str());

			const char* types[] = { "SSR", "SR", "R" };
			for (int i = 0; i < 3; i++) {
				auto cards = skills["Skills"][types[i]];

				for (auto& card : cards.items()) {
					auto name = card.key();
					auto events = card.value()["Events"];
					SupportCard skill;

					skill.Name = utility::ConvertUtf8ToUtf16(name.c_str());

					for (auto &e : events) {
						for (auto& choise : e.items()) {
							SupportCard::Event event;

							for (auto& option : choise.value()) {
								SupportCard::Choise choise;

								choise.Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
								choise.Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
								event.Choises.push_back(choise);
							}
							
							std::wstring EventName = utility::ConvertUtf8ToUtf16(choise.key().c_str());
							skill.Events[EventName] = event;

							if (SkillMap.find(EventName) == SkillMap.end()) {
								SkillMap[EventName] = event;
							}
						}
					}

					Skills.push_back(skill);
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

void EventLibrary::InitDB()
{
	CreateDirectoryA(DBPath.c_str(), NULL);

	simstring::ngram_generator gen(3, false);
	simstring::writer_base<std::wstring> dbw(gen, DBPath + "events.db");

	for (auto& pair : SkillMap) {
		dbw.insert(pair.first);
	}
	dbw.close();
}

std::wstring EventLibrary::search(const std::wstring& name)
{
	simstring::reader dbr;
	
	dbr.open(DBPath + "events.db");

	std::vector<std::wstring> xstrs;
	dbr.retrieve(name, simstring::cosine, 0.6, std::back_inserter(xstrs));
	dbr.close();

	return !xstrs.empty() ? xstrs.front() : L"";
}
