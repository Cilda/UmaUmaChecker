#pragma once

#include <memory>
#include "EventRoot.h"

class EventLibrary
{
public:
	EventLibrary();
	~EventLibrary();

	void Clear();
	bool Load();

	std::shared_ptr<EventSource> RetrieveEvent(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveEventFromOptionTitle(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveCharaEvent(const std::wstring& name, const std::wstring& CharaName);
	std::shared_ptr<EventSource> RetrieveCharaEventFromOptionTitle(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveScenarioEvent(const std::wstring& name);
	std::shared_ptr<EventRoot> RetrieveCharaName(const std::wstring& name);

	EventRoot* GetCharacter(const std::wstring& name);

	const std::vector<std::vector<std::shared_ptr<EventRoot>>>& GetCharacters() const { return CharasByRank; }

private:
	bool LoadEvent();
	bool LoadChara();
	bool LoadScenarioEvent();
	bool LoadSkills();

	void InitEventDB();
	void InitCharaDB();
	void InitCharaNameDB();
	void InitScenarioEventDB();

	void DeleteDBFiles();
	std::wstring GetBestMatchString(const std::vector<std::wstring>& xstrs, const std::wstring& text);

private:
	// サポートカード
	std::vector<std::shared_ptr<EventRoot>> Events; // サポートカードイベント保持用
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> EventMap; // キー: イベント名, 値: 選択肢
	// キャライベント
	std::vector<std::shared_ptr<EventRoot>> Charas;
	std::vector<std::vector<std::shared_ptr<EventRoot>>> CharasByRank;
	std::unordered_map<std::wstring, std::shared_ptr<EventRoot>> CharaMap; // キャライベント保持用
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> CharaEventMap; // キー: イベント名, 値: 選択肢
	// シナリオイベント
	std::vector<std::shared_ptr<EventRoot>> ScenarioEvents; 
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> ScenarioEventMap; // キー: イベント名, 値: 選択肢

	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap; // キー: 選択肢, 値: イベントソース

	std::unordered_map<std::wstring, std::wstring> SkillMap; // スキル名 -> 説明

	std::string DBPath;
};

