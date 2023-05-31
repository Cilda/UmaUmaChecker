#include "ScrollbarDetector.h"


#include <opencv2/opencv.hpp>


ScrollbarDetector::ScrollbarDetector(cv::Mat& img)
{
	InitScrollInfo(img);
}

ScrollbarDetector::~ScrollbarDetector()
{
}

int ScrollbarDetector::GetPos() const
{
	return 0;
}

bool ScrollbarDetector::IsBegin() const
{
	return false;
}

bool ScrollbarDetector::IsEnd() const
{
	return false;
}

void ScrollbarDetector::InitScrollInfo(cv::Mat& img)
{
	TotalLength = img.size().height;
	Length = 0;

	cv::Mat scr;
	cv::inRange(img, cv::Scalar(140, 121, 123), cv::Scalar(168, 154, 156), scr);
}
