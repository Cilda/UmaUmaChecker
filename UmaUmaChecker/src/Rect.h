#pragma once

#include <nlohmann/json.hpp>

template<class T>
class Rect {
public:
	T x;
	T y;
	T width;
	T height;

	Rect()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

	Rect(T x, T y, T width, T height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Rect<double>, x, y, width, height);