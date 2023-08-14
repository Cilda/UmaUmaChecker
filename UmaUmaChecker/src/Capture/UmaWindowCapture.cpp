#include <Windows.h>
#include "UmaWindowCapture.h"

#include <gdiplus.h>

#include "Config/Config.h"
#include "../../../libwinrt/winrt_capture.h"


struct GraphicsCapture* UmaWindowCapture::capture = nullptr;

UmaWindowCapture::UmaWindowCapture()
{
}

UmaWindowCapture::~UmaWindowCapture()
{
	
}

void UmaWindowCapture::Uninitilize()
{
	if (capture) {
		free_winrt_capture(capture);
		capture = nullptr;
	}
}

HWND UmaWindowCapture::GetUmaWindow()
{
	return FindWindow(TEXT("UnityWndClass"), TEXT("umamusume"));
}

Gdiplus::Bitmap* UmaWindowCapture::ScreenShot()
{
	HWND hWnd = GetUmaWindow();
	if (!hWnd) return nullptr;

	switch (Config::GetInstance()->CaptureMode) {
		case CaptureMode::BitBlt: return GdiScreenShot(hWnd);
		case CaptureMode::WindowGraphicsCapture: return WinRTScreenShot(hWnd);
	}

	return nullptr;
}

Gdiplus::Bitmap* UmaWindowCapture::GdiScreenShot(HWND hWnd)
{
	RECT rc, rw;
	POINT pt = { 0, 0 };
	BITMAPINFO bmpinfo;
	byte* lpPixel;

	GetWindowRect(hWnd, &rw);
	GetClientRect(hWnd, &rc);

	if (rc.right == 0 || rc.bottom == 0) return nullptr;

	ClientToScreen(hWnd, &pt);

	ZeroMemory(&bmpinfo, sizeof(bmpinfo));
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = rc.right;
	bmpinfo.bmiHeader.biHeight = rc.bottom;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = 24;
	bmpinfo.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(NULL);
	HDC hdc_mem = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateDIBSection(hdc, &bmpinfo, DIB_RGB_COLORS, (void**)&lpPixel, NULL, 0);
	if (!hBmp) {
		DeleteDC(hdc_mem);
		ReleaseDC(hWnd, hdc);
		return nullptr;
	}

	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdc_mem, hBmp);
	BitBlt(hdc_mem, 0, 0, rc.right, rc.bottom, hdc, pt.x, pt.y, SRCCOPY);
	SelectObject(hdc_mem, hOldBmp);

	Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromHBITMAP(hBmp, NULL);

	DeleteDC(hdc_mem);
	DeleteObject(hBmp);
	ReleaseDC(hWnd, hdc);

	return image;
}

Gdiplus::Bitmap* UmaWindowCapture::WinRTScreenShot(HWND hWnd)
{
	if (!winrt_capture_is_supported()) return nullptr;

	RECT rect;

	GetClientRect(hWnd, &rect);

	if (!capture || winrt_capture_get_target(capture) != hWnd) {
		if (capture) free_winrt_capture(capture);
		capture = winrt_init_capture(hWnd);
	}

	return capture ? winrt_screenshot(capture) : nullptr;
}
