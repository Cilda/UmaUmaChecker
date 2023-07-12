#pragma once

#include <list>

#include "Point.h"

namespace cv {
	class Mat;
}

class ScrollbarDetector
{
public:
	ScrollbarDetector(cv::Mat& img);
	~ScrollbarDetector();

	bool IsValid() const;
	int GetPos() const;
	int GetTotalLength() const;
	int GetBarLength() const;
	int GetBarLengthRatio() const;
	bool IsBegin() const;
	bool IsEnd() const;

private:
	void InitScrollInfo(cv::Mat& img);

	std::list<Point<int>> GetMargin(cv::Mat& img);
	
private:
	static Point<double> Start;
	static Point<double> End;

private:
	bool valid;

	int TotalLength;
	int Length;

	int ScrollBarStartY;
	int ScrollBarEndY;

	int MinY;
	int MaxY;
};

