#pragma once

#include <vector>
#include <string>
#include <memory>
#include "EventOption.h"

class EventSource {
public:
	std::wstring Name; // �C�x���g��
	std::vector<std::shared_ptr<EventOption>> Options; // �I����
};

