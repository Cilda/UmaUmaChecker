#pragma once

template<typename T>
class Point
{
public:
	Point() {}
	Point(T x, T y) : x_(x), y_(y) {}

	T x() const { return x_; }
	T y() const { return y_; }

	void x(const T& x) { x_ = x; }
	void y(const T& y) { y_ = y; }

private:
	T x_, y_;
};

