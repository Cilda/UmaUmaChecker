#include "ScenarioData.h"

#include <sstream>
#include <fstream>

#include <nlohmann/json.hpp>
#include "simstring/simstring.h"
#include "utility.h"

using json = nlohmann::ordered_json;

ScenarioData::ScenarioData()
{
}

ScenarioData::~ScenarioData()
{
}

bool ScenarioData::Load(const std::wstring& path)
{
	std::fstream stream(path);
	if (!stream.good()) return false;

	try {
		json events = json::parse(stream);

		for (auto& scenario : events.items()) {
			std::shared_ptr<EventRoot> root(new EventRoot());

			for (auto& event : scenario.value().items()) {
				std::shared_ptr<EventSource> source(new EventSource());

				for (auto& option : event.value()) {
					std::shared_ptr<EventOption> event_option(new EventOption());

					event_option->Title = utility::from_u8string(option["Title"].get<std::string>());
					event_option->Effect = utility::from_u8string(option["Effect"].get<std::string>());

					source->Options.push_back(event_option);

					if (OptionMap.find(event_option->Title) == OptionMap.end()) {
						OptionMap[event_option->Title] = source;
					}
				}

				std::wstring EventName = utility::from_u8string(event.key());
				source->Name = EventName;
				root->Events[EventName] = source;
				EventMap[EventName] = source;
			}

			root->Name = utility::from_u8string(scenario.key());
			EventRoots.push_back(root);
		}
	}
	catch (json::exception& ex) {
		return false;
	}

	std::filesystem::path base_path = path;
	std::filesystem::path dbpath_parent = utility::GetExeDirectory() + L"\\simstring\\";

	InitDB(dbpath_parent / base_path.stem() / L"");

	return true;
}

std::shared_ptr<EventSource> ScenarioData::RetrieveTitle(const std::wstring& title, EventRoot* root)
{
	if (title.empty()) return nullptr;

	std::vector<std::wstring> xstrs;

	for (double ratio = 100; ratio > 40; ratio -= 10) {
		titlereader->retrieve(title, simstring::cosine, ratio / 100.0, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();

	auto event = EventMap.find(match);
	if (event == EventMap.end()) return nullptr;

	return event->second;
}

std::shared_ptr<EventSource> ScenarioData::RetrieveOption(const std::wstring& option, EventRoot* root)
{
	std::vector<std::wstring> xstrs;

	for (double ratio = 100; ratio > 40; ratio -= 10) {
		optionreader->retrieve(option, simstring::cosine, ratio / 100.0, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();

	const auto& event = OptionMap.find(match);
	if (event == OptionMap.end()) return nullptr;

	return event->second;
}

void ScenarioData::InitDB(const std::filesystem::path& path)
{
	this->dbpath = path / L"event.db";
	this->optiondbpath = path / L"option.db";

	CreateDirectoryW((wchar_t*)path.parent_path().parent_path().c_str(), NULL);
	CreateDirectoryW((wchar_t*)path.parent_path().c_str(), NULL);

	simstring::ngram_generator gen_event(2, false);
	simstring::writer_base<std::wstring> dbw_event(gen_event, dbpath.string());

	simstring::ngram_generator gen_option(2, false);
	simstring::writer_base<std::wstring> dbw_option(gen_option, optiondbpath.string());

	for (auto& scenario : EventRoots) {
		for (auto& event : scenario->Events) {
			dbw_event.insert(event.first); // イベント名
			for (auto& option : event.second->Options) {
				dbw_option.insert(option->Title);
			}
		}
	}

	dbw_event.close();
	dbw_option.close();

	titlereader = std::shared_ptr<simstring::reader>(new simstring::reader());
	optionreader = std::shared_ptr<simstring::reader>(new simstring::reader());

	titlereader->open(dbpath.string());
	optionreader->open(optiondbpath.string());
}
