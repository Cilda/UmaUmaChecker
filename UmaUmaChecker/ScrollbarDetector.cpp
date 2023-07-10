#include "ScrollbarDetector.h"


#include <opencv2/opencv.hpp>


ScrollbarDetector::ScrollbarDetector(cv::Mat& img) : valid(false)
{
	InitScrollInfo(img);
}

ScrollbarDetector::~ScrollbarDetector()
{
}

bool ScrollbarDetector::IsValid() const
{
	return valid;
}

int ScrollbarDetector::GetPos() const
{
	return valid ? ScrollBarStartY : 0;
}

int ScrollbarDetector::GetTotalLength() const
{
	return valid ? TotalLength : 0;
}

int ScrollbarDetector::GetBarLength() const
{
	return valid ? Length : 0;
}

int ScrollbarDetector::GetBarLengthRatio() const
{
	return valid ? (double)Length / TotalLength * 100 : 0;
}

bool ScrollbarDetector::IsBegin() const
{
	return valid && ScrollBarStartY == MinY;
}

bool ScrollbarDetector::IsEnd() const
{
	return valid && ScrollBarEndY == MaxY;
}

void ScrollbarDetector::InitScrollInfo(cv::Mat& img)
{
	TotalLength = 0;
	Length = 0;

	ScrollBarStartY = ScrollBarEndY = -1;

	cv::Mat bar, scr;
	cv::inRange(img, cv::Scalar(140, 121, 123), cv::Scalar(180, 179, 189), bar);
	cv::inRange(img, cv::Scalar(140, 121, 123), cv::Scalar(217, 210, 211), scr);

	MinY = -1;
	MaxY = -1;
	for (int y = 0; y < scr.size().height; y++) {
		for (int x = 0; x < scr.size().width; x++) {
			cv::uint8_t b = scr.at<cv::uint8_t>(y, x);
			if (b == 255) {
				if (MinY == -1) MinY = y;
				if (y > MaxY) MaxY = y;
			}
		}
	}

	if (MinY == -1 || MaxY == -1) return;

	TotalLength = MaxY - MinY;

	for (int y = MinY; y <= MaxY; y++) {
		cv::uint8_t c = 0;

		for (int x = 0; x < bar.size().width && c != 255; x++) c = bar.at<cv::uint8_t>(y, x);

		if (c == 255 && ScrollBarStartY == -1) {
			ScrollBarStartY = y;
		}
		else if (c == 0 && ScrollBarStartY != -1 && ScrollBarEndY == -1) {
			ScrollBarEndY = y;
			break;
		}
	}

	if (ScrollBarStartY == -1 && ScrollBarEndY == -1) {
		return;
	}

	if (ScrollBarEndY == -1) ScrollBarEndY = MaxY;

	Length = ScrollBarEndY - ScrollBarStartY;
	valid = true;
}
