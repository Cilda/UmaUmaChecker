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

