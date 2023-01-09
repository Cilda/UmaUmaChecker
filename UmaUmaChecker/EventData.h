#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "EventRoot.h"

class EventData
{
public:
	EventData();
	~EventData();

	bool Load(const std::wstring& path);

	std::shared_ptr<EventSource> RetrieveTitle(const std::wstring& title, EventRoot* root = nullptr);
	std::shared_ptr<EventSource> RetrieveOption(const std::wstring& option);
	std::shared_ptr<EventRoot> RetrieveName(const std::wstring& name);
	std::shared_ptr<EventRoot> GetName(const std::wstring& name);

	const std::vector<std::vector<std::shared_ptr<EventRoot>>>& GetRanks() const { return ByRank; }

private:
	void InitDB(const std::filesystem::path& path);
	std::wstring GetBestMatchString(const std::vector<std::wstring>& xstrs, const std::wstring& text);

private:
	std::vector<std::shared_ptr<EventRoot>> EventRoots;
	std::vector<std::vector<std::shared_ptr<EventRoot>>> ByRank;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> EventMap;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap;
	std::unordered_map<std::wstring, std::shared_ptr<EventRoot>> NameMap;

	std::filesystem::path dbpath;
	std::filesystem::path optiondbpath;
	std::filesystem::path namedbpath;
};