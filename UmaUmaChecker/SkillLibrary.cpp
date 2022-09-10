#include <string>

#include "SkillLibrary.h"
#include "utility.h"

#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

SkillLibrary::SkillLibrary()
{
}

SkillLibrary::~SkillLibrary()
{
}

bool SkillLibrary::Load()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Skills.json");
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
					Skill skill;

					skill.Name = utility::ConvertUtf8ToUtf16(name.c_str());

					for (auto &e : events) {
						for (auto& choise : e.items()) {
							Skill::Event event;

							for (auto& option : choise.value()) {
								Skill::Choise choise;

								choise.Title = utility::ConvertUtf8ToUtf16(option["Title"].get<std::string>().c_str());
								choise.Effect = utility::ConvertUtf8ToUtf16(option["Effect"].get<std::string>().c_str());
								event.Choises.push_back(choise);
							}
							
							std::wstring EventName = utility::ConvertUtf8ToUtf16(choise.key().c_str());
							skill.Events[EventName] = event;
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
