#pragma once

#include <opencv2/opencv.hpp>

class EventDetector
{
public:
	enum EventType {
		Undefined,
		SupportCard,
		Character,
		Scenario,
	};

public:
	EventDetector(cv::Mat& image);

	EventType GetEventType();

private:
	bool IsCardEvent(const cv::Mat& srcImg);
	bool IsCharaEvent(const cv::Mat& srcImg);
	bool IsScenarioEvent(const cv::Mat& srcImg);

private:
	cv::Mat& image;
	bool IsDetected;
	EventType Type;
};

