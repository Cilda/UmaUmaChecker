#pragma once

#include "SupportCard.h"

class EventLibrary
{
public:
	EventLibrary();
	~EventLibrary();

	bool Load();
	bool LoadEvent();
	bool LoadChara();
	void InitEventDB();
	void InitCharaDB();

	std::wstring SearchEvent(const std::wstring& name);
	std::wstring SearchCharaEvent(const std::wstring& name);

public:
	std::vector<SupportCard> Events;
	std::vector<SupportCard> Charas;
	std::unordered_map<std::wstring, SupportCard::Event> EventMap;
	std::unordered_map<std::wstring, SupportCard::Event> CharaMap;

	std::string DBPath;
};

