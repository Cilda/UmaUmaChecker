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
	return StartY;
}

int ScrollbarDetector::GetTotalLength() const
{
	return TotalLength;
}

int ScrollbarDetector::GetBarLength() const
{
	return Length;
}

int ScrollbarDetector::GetBarLengthRatio() const
{
	return std::round((double)Length / TotalLength * 100);
}

bool ScrollbarDetector::IsBegin() const
{
	return StartY == 0;
}

bool ScrollbarDetector::IsEnd() const
{
	return EndY == TotalLength;
}

void ScrollbarDetector::InitScrollInfo(cv::Mat& img)
{
	TotalLength = img.size().height;
	Length = 0;

	StartY = EndY = -1;

	cv::Mat scr;
	cv::inRange(img, cv::Scalar(140, 121, 123), cv::Scalar(180, 179, 189), scr);

	for (int y = 0; y < scr.size().height; y++) {
		cv::uint8_t c = scr.at<cv::uint8_t>(y, 2);
		if (c == 255 && StartY == -1) {
			StartY = y;
		}
		else if (c == 0 && StartY != -1 && EndY == -1) {
			EndY = y - 1;
			break;
		}
	}

	if (StartY == -1 && EndY == -1) {
		return;
	}

	if (EndY == -1) EndY = scr.size().height;

	Length = EndY - StartY;
}
