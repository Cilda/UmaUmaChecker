#pragma once

#include <memory>
#include "Character.h"

class EventLibrary
{
public:
	EventLibrary();
	~EventLibrary();

	void Clear();
	bool Load();
	bool LoadEvent();
	bool LoadChara();
	void InitEventDB();
	void InitCharaDB();

	std::wstring SearchEvent(const std::wstring& name);
	std::wstring SearchEventFromChoise(const std::wstring& name);
	std::wstring SearchCharaEvent(const std::wstring& name);

public:
	std::vector<std::shared_ptr<Character>> Events;
	std::vector<std::shared_ptr<Character>> Charas;
	std::unordered_map<std::wstring, Character::Event> EventMap;
	std::unordered_map<std::wstring, std::shared_ptr<Character>> CharaMap;
	std::unordered_map<std::wstring, Character::Event> CharaEventMap;
	std::unordered_map<std::wstring, std::shared_ptr<Character>> ChoiseMap;

	std::string DBPath;
};

