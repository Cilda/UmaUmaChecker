#pragma once

#include "SupportCard.h"

class EventLibrary
{
public:
	EventLibrary();
	~EventLibrary();

	bool Load();
	void InitDB();

	std::wstring search(const std::wstring& name);

public:
	std::vector<SupportCard> Skills;
	std::unordered_map<std::wstring, SupportCard::Event> SkillMap;

	std::string DBPath;
};

