#pragma once

#include <memory>
#include "Event.h"

#include "EventData.h"
#include "ScenarioData.h"

namespace simstring {
	class reader;
}

class EventLibrary
{
public:
	EventLibrary();
	~EventLibrary();

	void Clear();
	bool Load();

private:
	void DeleteDBFiles();
	bool LoadSkills();

public:
	EventData CardEvent;
	EventData CharaEvent;
	ScenarioData ScenarioEvent;
	std::unordered_map<std::wstring, std::wstring> SkillMap; // スキル名 -> 説明

private:
	std::string DBPath;
};

extern EventLibrary EventLib;