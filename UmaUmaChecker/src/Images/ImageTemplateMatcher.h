#pragma once

#include <opencv2/opencv.hpp>

class ImageTemplateMatcher
{
public:
	ImageTemplateMatcher(const cv::Mat& templateImage);

	double Match(const cv::Mat& targetImage);
	cv::Point GetMatchPoint() const { return matchPoint; }

private:
	const cv::Mat& templateImage;
	cv::Point matchPoint;
};

