#pragma once

#include <memory>
#include "Event.h"

#include "EventData.h"
#include "ScenarioData.h"

namespace simstring {
	class reader;
}

class EventLibrary
{
public:
	EventLibrary();
	~EventLibrary();

	void Clear();
	bool Load();

private:
	void DeleteDBFiles();

public:
	EventData CardEvent;
	EventData CharaEvent;
	ScenarioData ScenarioEvent;

private:
	std::string DBPath;
};
