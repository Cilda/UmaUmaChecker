#pragma once

#include <memory>
#include "EventRoot.h"

#include "EventData.h"

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

	
	std::shared_ptr<EventSource> RetrieveScenarioEvent(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveScenarioEventFromOptionTitle(const std::wstring& name);

	EventRoot* GetCharacter(const std::wstring& name);

	const std::vector<std::vector<std::shared_ptr<EventRoot>>>& GetCharacters() const { return CharaEvent.GetRanks(); }

private:
	bool LoadScenarioEvent();
	void InitScenarioEventDB();

	void DeleteDBFiles();
	std::wstring GetBestMatchString(const std::vector<std::wstring>& xstrs, const std::wstring& text);

	void Retrieve(simstring::reader& dbr, double MinRatio, const std::wstring& name, std::vector<std::wstring>& xstrs);
public:
	EventData CardEvent;
	EventData CharaEvent;

private:
	// シナリオイベント
	std::vector<std::shared_ptr<EventRoot>> ScenarioEvents; 
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> ScenarioEventMap; // キー: イベント名, 値: 選択肢

	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap; // キー: 選択肢, 値: イベントソース

	std::string DBPath;

private:
	static const double MIN_DEFAULT_RATIO;
	static const double MIN_CHARA_RATIO;
};
