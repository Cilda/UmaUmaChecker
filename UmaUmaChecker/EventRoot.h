#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "EventSource.h"

class EventRoot
{
public:
	
public:
	std::wstring Name;
	std::unordered_map<std::wstring, std::shared_ptr<EventSource>> Events;
};

