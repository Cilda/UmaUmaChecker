#pragma once

#include <vector>
#include <string>
#include <memory>
#include "EventOption.h"

class EventSource {
public:
	std::wstring Name; // イベント名
	std::vector<std::shared_ptr<EventOption>> Options; // 選択肢
};

