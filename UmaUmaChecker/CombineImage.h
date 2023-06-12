#pragma once

#include <vector>
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

	int GetProgressTime() const { return msec; }

	bool IsCapturing() const { return IsCapture; }
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
	int BarLength;
	int msec;

	std::vector<CombineImageInfo> DetectedYLines;
	std::vector<cv::Mat> Images;

};

