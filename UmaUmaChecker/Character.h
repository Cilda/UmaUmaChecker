#pragma once

#include <vector>
#include <unordered_map>
#include <string>


class Character
{
public:
	class Choise {
	public:
		std::wstring Title;
		std::wstring Effect;
	};

	class Event {
	public:
		std::vector<Choise> Choises;
	};
public:
	std::wstring Name;
	std::unordered_map<std::wstring, Event> Events;
};

