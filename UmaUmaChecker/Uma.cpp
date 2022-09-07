#include <Windows.h>
#include <gdiplus.h>

#include "Uma.h"


#include <codecvt>
#include "utility.h"


const cv::Rect2d Uma::CharaEventBound = { 0.1545, 0.1884, 0.5472, 0.03140 };


Uma::Uma()
{
	bDetected = false;
	thread = nullptr;
	api = new tesseract::TessBaseAPI();

	std::wstring name = utility::GetExeDirectory();
	std::wstring dir = name + L"\\tessdata";

	api->Init(utility::to_string(dir).c_str(), "jpn");
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

	bStop = false;
	thread = new std::thread(&Uma::MonitorThread, this);
}

void Uma::Stop()
{
	if (!thread) return;

	bStop = true;
	thread->join();
	delete thread;
	thread = nullptr;
}

cv::Mat Uma::BitmapToCvMat(Gdiplus::Bitmap* image)
{
	//assert(image->GetPixelFormat() == PixelFormat32bppRGB);

	Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());
	Gdiplus::BitmapData data;
	image->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, image->GetPixelFormat(), &data);

	cv::Mat mat = cv::Mat(image->GetHeight(), image->GetWidth(), CV_8UC4, data.Scan0, data.Stride);

	image->UnlockBits(&data);

	return mat;
}

cv::Mat Uma::ImageBinarization(cv::Mat& srcImg)
{
	cv::Mat gray;
	cv::Mat bin;

	cv::cvtColor(srcImg, gray, cv::COLOR_BGR2GRAY);
	cv::threshold(gray, bin, 236, 255, cv::THRESH_BINARY_INV);

	return bin;
}

void Uma::MonitorThread()
{
	while (!bStop) {
		Gdiplus::Bitmap* image = ScreenShot();
		if (image) {
			api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
			
			cv::Mat mat = BitmapToCvMat(image);
			cv::Mat cut = cv::Mat(mat, cv::Rect(
				Uma::CharaEventBound.x * mat.size().width,
				Uma::CharaEventBound.y * mat.size().height,
				Uma::CharaEventBound.width * mat.size().width,
				Uma::CharaEventBound.height * mat.size().height
			));
			cv::Mat rsImg;

			cv::resize(cut, rsImg, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);
			cv::Mat bin = Uma::ImageBinarization(rsImg);
				
			api->SetImage(bin.data, bin.size().width, bin.size().height, bin.channels(), bin.step1());
			api->Recognize(NULL);

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
			std::wstring text = convert.from_bytes(api->GetUTF8Text());
			text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

			delete image;
		}

		Sleep(100);
	}
}
