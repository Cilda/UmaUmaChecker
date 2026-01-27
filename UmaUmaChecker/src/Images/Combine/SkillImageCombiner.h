#pragma once

#include <vector>
#include <filesystem>
#include <string>

#include <opencv2/opencv.hpp>

class SkillImageCombiner
{
public:
	SkillImageCombiner();
	~SkillImageCombiner();

	bool Process(bool IsDestroy);
	bool Save(const std::filesystem::path& path) const;
	void Reset();

	bool IsScanStarted() const { return ScrollOnTop; }
	std::wstring GetError() const { return message; }
	bool HasImages() const { return !frames.empty(); }

private:
	cv::Mat CutTemplateImage(const cv::Mat& src, int* detectY);
	void SetError(const std::wstring& message);

private:
	std::vector<int> detectVerticalPositions; // for debug
	std::vector<cv::Mat> frames;
	int width, height;
	cv::Mat prevImg;
	cv::Mat templateImage;
	int minDetectVerticalPosition;
	bool ScrollOnTop;
	int processedScrollPos;
	bool hasError;
	std::wstring message;
};

