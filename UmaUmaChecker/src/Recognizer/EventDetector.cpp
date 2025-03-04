#include "EventDetector.h"

EventDetector::EventDetector(cv::Mat& image) : image(image), IsDetected(false)
{
}

EventDetector::EventType EventDetector::GetEventType()
{
	if (!IsDetected) {
		if (IsCardEvent(image)) Type = SupportCard;
		else if (IsCharaEvent(image)) Type = Character;
		else if (IsScenarioEvent(image)) Type = Scenario;
		else Type = Undefined;

		IsDetected = true;
	}

	return Type;
}

bool EventDetector::IsCardEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(240, 145, 40), cv::Scalar(255, 210, 120), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

bool EventDetector::IsCharaEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(150, 100, 251), cv::Scalar(209, 198, 255), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

bool EventDetector::IsScenarioEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(20, 200, 110), cv::Scalar(80, 240, 170), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}