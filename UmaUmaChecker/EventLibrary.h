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

	bool LoadEvent();
	bool LoadChara();
	bool LoadScenarioEvent();

	void InitEventDB();
	void InitCharaDB();
	void InitScenarioEventDB();

	std::shared_ptr<EventSource> RetrieveEvent(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveEventFromOptionTitle(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveCharaEvent(const std::wstring& name, const std::wstring& CharaName);
	std::shared_ptr<EventSource> RetrieveCharaEventFromOptionTitle(const std::wstring& name);
	std::shared_ptr<EventSource> RetrieveScenarioEvent(const std::wstring& name);
	EventRoot* GetCharacter(const std::wstring& name);

	const std::vector<std::vector<std::shared_ptr<EventRoot>>>& GetCharacters() const { return CharasByRank; }

private:
	void DeleteDBFiles();

private:
	std::vector<std::shared_ptr<EventRoot>> Events; // �T�|�[�g�J�[�h�C�x���g�ێ��p
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> EventMap; // �L�[: �C�x���g��, �l: �I����

	std::vector<std::shared_ptr<EventRoot>> Charas;
	std::vector<std::vector<std::shared_ptr<EventRoot>>> CharasByRank;
	std::unordered_map<std::wstring, std::shared_ptr<EventRoot>> CharaMap; // �L�����C�x���g�ێ��p
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> CharaEventMap; // �L�[: �C�x���g��, �l: �I����

	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap; // �L�[: �I����, �l: �C�x���g�\�[�X

	std::vector<std::shared_ptr<EventRoot>> ScenarioEvents; // �V�i���I�C�x���g
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> ScenarioEventMap; // �L�[: �C�x���g��, �l: �I����

	std::string DBPath;
};

