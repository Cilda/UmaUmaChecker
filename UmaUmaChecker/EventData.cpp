#include "EventData.h"

#include <sstream>
#include <fstream>
#include <regex>

#include <nlohmann/json.hpp>
#include "simstring/simstring.h"

#include "utility.h"

using json = nlohmann::ordered_json;

EventData::EventData()
{
}

EventData::~EventData()
{
}

bool EventData::Load(const std::wstring& path)
{
	std::fstream stream(path);
	if (!stream.good()) return false;

	try {
		json skills = json::parse(stream);

		for (auto& cards : skills.items().begin().value()) {
			std::vector<std::shared_ptr<EventRoot>> RankList;

			for (auto& card : cards.items()) {
				auto name = card.key();
				auto events = card.value()["Events"];
				std::shared_ptr<EventRoot> skill(new EventRoot());

				skill->Name = utility::from_u8string(name);

				for (auto& e : events) {
					for (auto& choise : e.items()) {
						std::shared_ptr<EventSource> event(new EventSource());

						for (auto& option : choise.value()) {
							std::shared_ptr<EventOption> choise(new EventOption());

							choise->Title = utility::from_u8string(option["Title"].get<std::string>());
							choise->Effect = utility::from_u8string(option["Effect"].get<std::string>());
							event->Options.push_back(choise);

							if (OptionMap.find(choise->Title) == OptionMap.end()) {
								OptionMap[choise->Title] = event;
							}
						}

						std::wstring EventName = utility::from_u8string(choise.key());
						event->Name = EventName;
						skill->Events[EventName] = event;

						if (EventMap.find(EventName) == EventMap.end()) {
							EventMap[EventName] = event;
						}
					}
				}

				EventRoots.push_back(skill);
				RankList.push_back(skill);

				NameMap[skill->Name] = skill;
			}

			std::sort(RankList.begin(), RankList.end(), [](std::shared_ptr<EventRoot> a, std::shared_ptr<EventRoot> b) {
				std::wregex regex(L"^［(.+?)］(.+?)$");
				std::wcmatch m1, m2;

				std::regex_search(a->Name.c_str(), m1, regex);
				std::regex_search(b->Name.c_str(), m2, regex);

				return m1[2].str() < m2[2].str();
			});

			ByRank.push_back(RankList);
			RankList.clear();
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

std::shared_ptr<EventSource> EventData::RetrieveTitle(const std::wstring& title, EventRoot* root)
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
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, title);
	}

	if (root) {
		auto event = root->Events.find(match);
		if (event == root->Events.end()) return nullptr;

		return event->second;
	}
	else {
		auto event = EventMap.find(match);
		if (event == EventMap.end()) return nullptr;

		return event->second;
	}
}

std::shared_ptr<EventSource> EventData::RetrieveOption(const std::wstring& option, EventRoot* root)
{
	std::vector<std::wstring> xstrs;

	for (double ratio = 100; ratio > 40; ratio -= 10) {
		optionreader->retrieve(option, simstring::cosine, ratio / 100.0, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, option);
	}

	const auto& event = OptionMap.find(match);
	if (event == OptionMap.end()) return nullptr;
	else if (root) {
		const auto exactEvent = root->Events.find(event->second->Name);
		if (exactEvent != root->Events.end()) return exactEvent->second;
	}

	return event->second;
}

std::shared_ptr<EventRoot> EventData::RetrieveName(const std::wstring& name)
{
	std::vector<std::wstring> xstrs;

	for (double ratio = 100; ratio > 20; ratio -= 10) {
		namereader->retrieve(name, simstring::cosine, ratio / 100.0, std::back_inserter(xstrs));
		if (xstrs.size() > 0)
			break;
	}

	if (xstrs.empty()) return nullptr;

	std::wstring match = xstrs.front();
	if (xstrs.size() >= 2) {
		match = GetBestMatchString(xstrs, name);
	}

	const auto& itr = NameMap.find(match);
	if (itr != NameMap.end()) {
		return itr->second;
	}

	return nullptr;
}

std::shared_ptr<EventRoot> EventData::GetName(const std::wstring& name)
{
	auto itr = NameMap.find(name);
	if (itr == NameMap.end()) return nullptr;

	return itr->second;
}

void EventData::InitDB(const std::filesystem::path& path)
{
	this->dbpath = path / L"event.db";
	this->optiondbpath = path / L"option.db";
	this->namedbpath = path / L"name.db";

	CreateDirectoryW((wchar_t*)path.parent_path().parent_path().c_str(), NULL);
	CreateDirectoryW((wchar_t*)path.parent_path().c_str(), NULL);

	simstring::ngram_generator gen_event(2, false);
	simstring::writer_base<std::wstring> dbw_event(gen_event, dbpath.string());

	simstring::ngram_generator gen_option(2, false);
	simstring::writer_base<std::wstring> dbw_option(gen_option, optiondbpath.string());

	simstring::ngram_generator gen_name(2, false);
	simstring::writer_base<std::wstring> dbw_name(gen_name, namedbpath.string());

	for (auto& source : EventMap) {
		dbw_event.insert(source.first);
	}

	for (auto& option : OptionMap) {
		if (!option.first.empty()) dbw_option.insert(option.first);
	}

	for (auto& root : EventRoots) {
		dbw_name.insert(root->Name);
	}

	dbw_event.close();
	dbw_option.close();
	dbw_name.close();

	titlereader = std::shared_ptr<simstring::reader>(new simstring::reader());
	optionreader = std::shared_ptr<simstring::reader>(new simstring::reader());
	namereader = std::shared_ptr<simstring::reader>(new simstring::reader());

	titlereader->open(dbpath.string());
	optionreader->open(optiondbpath.string());
	namereader->open(namedbpath.string());
}

std::wstring EventData::GetBestMatchString(const std::vector<std::wstring>& xstrs, const std::wstring& text)
{
	simstring::ngram_generator gen(1, false);

	double max_rate = 0.0;
	std::wstring best_match;
	std::vector<std::wstring> basengrams;

	gen(text, std::inserter(basengrams, basengrams.end()));

	for (auto& str : xstrs) {
		std::vector<std::wstring> ngrams;
		int total = 0, equal = 0;

		gen(str, std::inserter(ngrams, ngrams.end()));

		for (auto& word1 : basengrams) {
			for (auto& word2 : ngrams) {
				if (word1 == word2) {
					equal++;
				}
			}

			total++;
		}

		if (max_rate < (double)equal / total) {
			max_rate = (double)equal / total;
			best_match = str;
		}
	}

	return best_match;
}
