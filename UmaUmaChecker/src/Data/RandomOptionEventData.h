#pragma once

#include <string>
#include <unordered_set>
#include <filesystem>

class RandomOptionEventData
{
public:
	RandomOptionEventData();

	void Load(const std::filesystem::path& path);

	bool IsSupportCardRandom(const std::wstring& name) const;
	bool IsCharacterRandom(const std::wstring& name) const;
	bool IsScenarioRandom(const std::wstring& name) const;

private:
	std::unordered_set<std::wstring> SupportCards;
	std::unordered_set<std::wstring> Characters;
	std::unordered_set<std::wstring> Scenarios;
};

