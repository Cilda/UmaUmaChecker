#pragma once

#include "CaptureMode.h"
#include <memory>

namespace Gdiplus {
	class Bitmap;
}

class UmaWindowCapture
{
private:
	friend std::unique_ptr<UmaWindowCapture>::deleter_type;

	UmaWindowCapture();
	~UmaWindowCapture();

	UmaWindowCapture(const UmaWindowCapture&) = delete;
	UmaWindowCapture& operator=(const UmaWindowCapture&) = delete;

public:
	static UmaWindowCapture* GetInstance();

	HWND GetUmaWindow();
	Gdiplus::Bitmap* ScreenShot();

private:
	Gdiplus::Bitmap* GdiScreenShot(HWND hWnd);
	Gdiplus::Bitmap* WinRTScreenShot(HWND hWnd);

private:
	struct GraphicsCapture* capture;
	//CaptureMode CaptureMode;

private:
	static std::unique_ptr<UmaWindowCapture> instance;
};

