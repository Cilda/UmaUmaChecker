#pragma once

#include <thread>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

class Uma
{
public:
	Uma();
	~Uma();

	HWND GetUmaWindow();
	Gdiplus::Bitmap *ScreenShot();

	void Start();
	void Stop();

public:
	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	static cv::Mat ImageBinarization(cv::Mat & srcImg);
	
private:
	void MonitorThread();

public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界

private:
	bool bDetected;
	bool bStop;
	std::thread* thread;
	tesseract::TessBaseAPI* api;
};

