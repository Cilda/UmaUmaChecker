#pragma once

#include <thread>
#include <opencv2/opencv.hpp>

class Uma
{
public:
	Uma();
	~Uma();

	HWND GetUmaWindow();
	Gdiplus::Bitmap *ScreenShot();

	void Start();

private:
	cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	void MonitorThread();

private:
	bool bDetected;
	std::thread* thread;
};

