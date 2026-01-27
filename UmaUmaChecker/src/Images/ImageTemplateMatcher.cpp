#include "ImageTemplateMatcher.h"

ImageTemplateMatcher::ImageTemplateMatcher(const cv::Mat& templateImage) :
	templateImage(templateImage)
{
}

double ImageTemplateMatcher::Match(const cv::Mat& targetImage)
{
	cv::Mat result;
	double MaxVal = 0;
	cv::Mat gray, grayTemp;

	cv::cvtColor(targetImage, gray, cv::COLOR_BGR2GRAY);
	cv::cvtColor(templateImage, grayTemp, cv::COLOR_BGR2GRAY);

	cv::matchTemplate(gray, grayTemp, result, cv::TM_CCOEFF_NORMED);
	cv::minMaxLoc(result, NULL, &MaxVal, NULL, &matchPoint);

	double RoundedMaxVal = std::round(MaxVal * 100.0) / 100.0;

	return RoundedMaxVal;
}
