#pragma once

#include <vector>

#include "Point.h"
#include "Range.h"

namespace cv {
	class Mat;
}

class ScrollbarDetector
{
public:
	ScrollbarDetector(const cv::Mat& img);
	~ScrollbarDetector();

	bool IsValid() const;
	int GetPos() const;
	int GetTotalLength() const;
	int GetBarLength() const;
	int GetBarLengthRatio() const;
	bool IsBegin() const;
	bool IsEnd() const;

private:
	void DetectScrollbar(const cv::Mat& img);

	std::vector<Point<int>> GetMargin(const cv::Mat& img);
	std::vector<Point<int>> GetScrollBar(const cv::Mat& img, const Point<int>& start, const Point<int>& end);
	
private:
	static Point<double> Start;
	static Point<double> End;

private:
	bool valid;

	Point<int> ScrollStart;
	Point<int> ScrollEnd;

	Range<Point<int>> ScrollRange;
	Range<Point<int>> BarRange;
};

