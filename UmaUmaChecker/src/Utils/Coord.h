#pragma once

#include <nlohmann/json.hpp>

class Coord
{
public:
	double x, y;

	Coord() : x(0), y(0) {}

	friend void from_json(const nlohmann::json& j, Coord& coord)
	{
		j.at("x").get_to(coord.x);
		j.at("y").get_to(coord.y);
	}

	friend void to_json(nlohmann::json& j, const Coord& coord)
	{
		j = nlohmann::json{ {"x", coord.x}, {"y", coord.y} };
	}
};