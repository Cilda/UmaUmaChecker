#pragma once

#include <vector>
#include <thread>

#include <opencv2/opencv.hpp>

struct CombineImageInfo {
	int StartY;
	int NextDetectedY;

	CombineImageInfo(int StartY, int NextDetectedY) {
		this->StartY = StartY;
		this->NextDetectedY = NextDetectedY;
	}
};

class CombineImage
{
public:
	CombineImage();
	~CombineImage();

	bool IsCapturing() const { return thread.joinable(); };
	void StartCapture();
	void EndCapture();

	bool Combine();

private:
	void _EndCapture();
	void Capture();

	int GetTemplateImage(const cv::Mat& mat, cv::Mat& cut);
	void CutScrollbar(const cv::Mat& src, cv::Mat& out);

private:
	bool IsCapture;
	bool IsScanStarted;
	bool IsFirstScan;
	cv::Mat TemplateImage;
	cv::Mat PrevImage;
	int CurrentScrollPos;
	int DetectedY;
	std::thread thread;
	int BarLength;

	std::vector<CombineImageInfo> DetectedYLines;
	std::vector<cv::Mat> Images;

};

