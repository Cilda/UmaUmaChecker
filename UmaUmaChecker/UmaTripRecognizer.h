#pragma once

#include <string>

#include <opencv2/opencv.hpp>

class UmaTripRecognizer
{
public:
	UmaTripRecognizer();
	virtual ~UmaTripRecognizer();

	bool Detect(cv::Mat& src, std::vector<std::wstring>& vec);

	static bool IsDialog(cv::Mat& src);

private:
	static cv::Rect2d range;
	static cv::Rect2d TitleRange;
	static double StartXRatio;
	static double WidthRatio;
	static double HeightRatio;
};

