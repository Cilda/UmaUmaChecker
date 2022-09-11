#pragma once

#include "EventLibrary.h"

#include <thread>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#define WM_CHANGEUMAEVENT (WM_USER + 1)

class Uma
{
public:
	Uma();
	~Uma();

	void Init();

	HWND GetUmaWindow();
	Gdiplus::Bitmap *ScreenShot();

	void Start();
	void Stop();

	void SetNotifyTarget(HWND hWnd);

public:
	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	static cv::Mat ImageBinarization(const cv::Mat& srcImg);
	
private:
	void MonitorThread();

	bool IsCharaEvent(const cv::Mat& srcImg);
	std::wstring GetCharaEventText(const cv::Mat& srcImg);

	bool IsCardEvent(const cv::Mat& srcImg);
	std::wstring GetCardEventText(const cv::Mat& srcImg);

public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界
	static const cv::Rect2d CardEventBound;

public:
	std::wstring EventName;
	SupportCard::Event* CurrentEvent;

private:
	bool bDetected;
	bool bStop;
	std::thread* thread;
	tesseract::TessBaseAPI* api;
	HWND hTargetWnd;
	EventLibrary SkillLib;
	
};

