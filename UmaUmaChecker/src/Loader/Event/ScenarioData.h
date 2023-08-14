#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "Event.h"

namespace simstring {
	class reader;
}

class ScenarioData : public BaseData
{
public:
	ScenarioData();
	~ScenarioData();

	bool Load(const std::wstring& path);

	std::shared_ptr<EventSource> RetrieveTitle(const std::wstring& title, EventRoot* root = nullptr) override;
	std::shared_ptr<EventSource> RetrieveOption(const std::wstring& option, EventRoot* root = nullptr) override;

private:
	void InitDB(const std::filesystem::path& path);

private:
	std::vector<std::shared_ptr<EventRoot>> EventRoots;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> EventMap;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap;

	std::filesystem::path dbpath;
	std::filesystem::path optiondbpath;

	std::shared_ptr<simstring::reader> titlereader;
	std::shared_ptr<simstring::reader> optionreader;
};

