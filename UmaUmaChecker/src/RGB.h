#pragma once

#include <nlohmann/json.hpp>

class RGB
{
public:
	float r;
	float g;
	float b;

public:
	RGB() : r(0), g(0), b(0)
	{
	}
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RGB, r, g, b);