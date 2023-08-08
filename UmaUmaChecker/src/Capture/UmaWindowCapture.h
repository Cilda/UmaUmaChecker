#pragma once

#include "CaptureMode.h"
#include <memory>

namespace Gdiplus {
	class Bitmap;
}

class UmaWindowCapture
{
private:
	UmaWindowCapture();
	~UmaWindowCapture();

	UmaWindowCapture(const UmaWindowCapture&) = delete;
	UmaWindowCapture& operator=(const UmaWindowCapture&) = delete;

public:
	static void Uninitilize();

	static HWND GetUmaWindow();
	static Gdiplus::Bitmap* ScreenShot();

private:
	static Gdiplus::Bitmap* GdiScreenShot(HWND hWnd);
	static Gdiplus::Bitmap* WinRTScreenShot(HWND hWnd);

private:
	static struct GraphicsCapture* capture;
};

