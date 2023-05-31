#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class EventOption
{
public:
	std::wstring Title;
	std::wstring Effect;
};

class EventSource
{
public:
	std::wstring Name; // イベント名
	std::vector<std::shared_ptr<EventOption>> Options; // 選択肢
};

class EventRoot
{
public:
	std::wstring Name;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> Events;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> OptionMap;
};


class BaseData {
public:
	virtual std::shared_ptr<EventSource> RetrieveTitle(const std::wstring& title, EventRoot* root = nullptr) = 0;
	virtual std::shared_ptr<EventSource> RetrieveOption(const std::wstring& option, EventRoot* root = nullptr) = 0;
};