#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <string>

struct CombineImageInfo {
	int StartY;
	int NextDetectedY;

	CombineImageInfo(int StartY, int NextDetectedY) {
		this->StartY = StartY;
		this->NextDetectedY = NextDetectedY;
	}
};

enum CombineStatus {
	Stop = 0, // 停止中
	WaitForMovingScrollbarOnTop, // スクロールバーが上部に移動するまで待機中
	Scanning, // スキャン中
	Combining, // 合成中
};

class CombineImage
{
public:
	CombineImage();
	~CombineImage();

	int GetProgressTime() const { return msec; }
	CombineStatus GetStatus() const { return status; }
	bool IsImageSaved() const { return IsSavedImage; }
	std::wstring GetError() const { return Error; }

	bool IsCapturing() const { return IsCapture; }
	void StartCapture();
	void EndCapture();

	bool Combine();

private:
	void _EndCapture(const std::wstring& error = L"");
	void Capture();
	void ProcessDetection(const cv::Mat& mat);

	int GetTemplateImage(const cv::Mat& mat, cv::Mat& cut);
	void CutRecognizeRange(const cv::Mat& mat, cv::Mat& out);
	void CutScrollbar(const cv::Mat& src, cv::Mat& out);

private:
	bool IsCapture;
	bool IsScanStarted;
	bool IsFirstScan;

	cv::Point RecognizePoint;
	cv::Mat TemplateImage;
	cv::Mat PrevImage;

	int CurrentScrollPos;
	int DetectedY;
	int BarLength;
	int msec;
	CombineStatus status;
	bool IsManualStop;
	bool IsSavedImage;

	std::wstring Error;

	std::vector<CombineImageInfo> DetectedYLines;
	std::vector<cv::Mat> Images;

};

