#pragma once

#include <nlohmann/json.hpp>

class Color
{
public:
    int r, g, b;

    Color() : r(0), g(0), b(0) {}

    friend void from_json(const nlohmann::json& j, Color& rgb)
    {
        j.at("r").get_to(rgb.r);
        j.at("g").get_to(rgb.g);
        j.at("b").get_to(rgb.b);
    }

    friend void to_json(nlohmann::json& j, const Color& rgb)
    {
        j = nlohmann::json{ {"r", rgb.r}, {"g", rgb.g}, {"b", rgb.b} };
    }
};