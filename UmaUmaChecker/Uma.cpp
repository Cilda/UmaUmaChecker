#include <Windows.h>
#include <gdiplus.h>

#include "Uma.h"


#include <tesseract/baseapi.h>
#include <codecvt>



Uma::Uma() : bDetected(false)
{
}

Uma::~Uma()
{
}

HWND Uma::GetUmaWindow()
{
	return FindWindow(TEXT("UnityWndClass"), TEXT("umamusume"));
}

Gdiplus::Bitmap *Uma::ScreenShot()
{
	HWND hWnd = GetUmaWindow();
	if (!hWnd) return nullptr;

	RECT rc, rw;
	POINT pt = { 0, 0 };

	GetWindowRect(hWnd, &rw);
	GetClientRect(hWnd, &rc);

	ClientToScreen(hWnd, &pt);

	HDC hdc = GetDC(NULL);
	HDC hdc_mem = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);

	SelectObject(hdc_mem, hBmp);
	BitBlt(hdc_mem, 0, 0, rc.right, rc.bottom, hdc, pt.x, pt.y, SRCCOPY);

	Gdiplus::Bitmap* image = new Gdiplus::Bitmap(hBmp, NULL);

	DeleteObject(hBmp);
	DeleteDC(hdc_mem);
	ReleaseDC(hWnd, hdc);

	return image;
}

void Uma::Start()
{
	HWND hWnd = GetUmaWindow();
	if (!hWnd || thread) return;

	thread = new std::thread(&Uma::MonitorThread, this);
}

cv::Mat Uma::BitmapToCvMat(Gdiplus::Bitmap* image)
{
	Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());
	Gdiplus::BitmapData data;
	image->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, image->GetPixelFormat(), &data);

	cv::Mat mat = cv::Mat(image->GetHeight(), image->GetWidth(), CV_8UC3, data.Scan0, data.Stride).clone();

	image->UnlockBits(&data);

	return mat;
}

void Uma::MonitorThread()
{
	while (true) {
		Gdiplus::Bitmap* image = ScreenShot();
		if (image) {
			tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
			char name[256];

			GetModuleFileNameA(NULL, name, 256);
			for (int i = lstrlenA(name) - 1; i >= 0; i--) {
				if (name[i] == '\\') {
					name[i] = '\0';
					break;
				}
			}

			std::string dir = (std::string(name) + "\\tessdata");

			if (!api->Init(dir.c_str(), "jpn")) {
				//api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
				//delete image;

				image = Gdiplus::Bitmap::FromFile(L"D:\\Users\\mile\\Documents\\Visual Studio 2022\\Projects\\UmaUmaChecker\\UmaUmaChecker\\test.png");
				cv::Mat mat = BitmapToCvMat(image);
				
				api->SetImage(mat.data, mat.size().width, mat.size().height, mat.channels(), mat.step1());
				api->Recognize(NULL);

				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
				const char* p = api->GetUTF8Text();
				std::wstring text = convert.from_bytes(api->GetUTF8Text());
			}

			delete image;
		}

		Sleep(1000);
	}
}
