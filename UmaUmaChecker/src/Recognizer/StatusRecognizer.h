#pragma once

#include <opencv2/core/core.hpp>

#include "Config/StatusRecognizerConfig.h"

class RecognizedStatus {
public:
	int Speed;
	int Stamina;
	int Power;
	int Guts;
	int Wisdom;

	int Total() {
		return Speed + Stamina + Power + Guts + Wisdom;
	}
} ;

class StatusRecognizer
{
public:
	StatusRecognizer(cv::Mat& img, StatusRecognizerConfig& config);

	bool IsValid() const { return this->Valid; }
	void Recognize();
	RecognizedStatus GetStatus() const { return Status; }

private:
	cv::Mat image;
	bool Valid;
	RecognizedStatus Status;
	StatusRecognizerConfig& Config;
};

