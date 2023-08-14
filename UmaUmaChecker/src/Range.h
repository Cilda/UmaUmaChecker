#pragma once

template<typename T>
class Range
{
public:
	Range() : start_(T()), end_(T()) {}
	Range(const T& start, const T& end) : start_(start), end_(end) {}

	T start() const { return start_; }
	T end() const { return end_; }

private:
	T start_;
	T end_;
};