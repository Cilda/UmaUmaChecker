#include "RandomOptionEventData.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "Utils/utility.h"

using json = nlohmann::json;

RandomOptionEventData::RandomOptionEventData()
{
}

void RandomOptionEventData::Load(const std::filesystem::path& path)
{
	try {
		auto type = json::parse(std::fstream(path));

		if (type["SupportCard"].is_array()) {
			for (auto& name : type["SupportCard"]) {
				SupportCards.insert(utility::from_u8string(name.get<std::string>()));
			}
		}

		if (type["Character"].is_array()) {
			for (auto& name : type["Character"]) {
				Characters.insert(utility::from_u8string(name.get<std::string>()));
			}
		}

		if (type["Scenario"].is_array()) {
			for (auto& name : type["Scenario"]) {
				Scenarios.insert(utility::from_u8string(name.get<std::string>()));
			}
		}

	}
	catch (std::exception& ex) {

	}
}

bool RandomOptionEventData::IsSupportCardRandom(const std::wstring& name) const
{
	return SupportCards.find(name) != SupportCards.end();
}

bool RandomOptionEventData::IsCharacterRandom(const std::wstring& name) const
{
	return Characters.find(name) != Characters.end();
}

bool RandomOptionEventData::IsScenarioRandom(const std::wstring& name) const
{
	return Scenarios.find(name) != Scenarios.end();
}
