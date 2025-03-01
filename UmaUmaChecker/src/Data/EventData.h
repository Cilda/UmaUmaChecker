#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "Event.h"

namespace simstring {
	class reader;
}

class EventData : public BaseData
{
public:
	EventData();
	~EventData();

	bool Load(const std::wstring& path, bool sort = false);

	std::shared_ptr<EventSource> RetrieveTitle(const std::wstring& title, EventRoot* root = nullptr) override;
	std::shared_ptr<EventSource> RetrieveOption(const std::wstring& option, EventRoot* root = nullptr) override;
	std::wstring RetrieveOptionTitle(const std::wstring& option, EventRoot* root = nullptr) override;
	std::shared_ptr<EventRoot> RetrieveName(const std::wstring& name);
	std::shared_ptr<EventRoot> GetName(const std::wstring& name);
	const std::vector<std::shared_ptr<EventRoot>>& GetRoots() const { return EventRoots; }

	bool IsEventNameDuplicate(const std::wstring& name);

	std::vector<std::wstring> GetAllEventNames();

private:
	void InitDB(const std::filesystem::path& path);
	std::wstring GetBestMatchString(const std::vector<std::wstring>& xstrs, const std::wstring& text);

private:
	std::vector<std::shared_ptr<EventRoot>> EventRoots;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> EventMap;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap;
	std::unordered_map<std::wstring, std::shared_ptr<EventRoot>> NameMap;
	std::unordered_map<std::wstring, int> EventDuplicationCount;

	std::filesystem::path dbpath;
	std::filesystem::path optiondbpath;
	std::filesystem::path namedbpath;

	std::shared_ptr<simstring::reader> titlereader;
	std::shared_ptr<simstring::reader> optionreader;
	std::shared_ptr<simstring::reader> namereader;
};