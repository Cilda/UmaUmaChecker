#pragma once

#include <string>
#include <opencv2/opencv.hpp>

class UmaTripRecognizer
{
public:
	UmaTripRecognizer();
	virtual ~UmaTripRecognizer();

	std::wstring Detect(cv::Mat& src);

private:
	static cv::Rect2d range;
};

