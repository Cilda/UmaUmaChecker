#include <Windows.h>
#include <gdiplus.h>

#include "Uma.h"


#include <codecvt>
#include <crtdbg.h>
#include "utility.h"


const cv::Rect2d Uma::CharaEventBound = { 0.1545, 0.1884, 0.6330, 0.03140 };
const cv::Rect2d Uma::CardEventBound = { 0.1532, 0.1876, 0.6293, 0.03030 };


Uma::Uma()
{
	bDetected = false;
	thread = nullptr;
	api = new tesseract::TessBaseAPI();

	std::wstring name = utility::GetExeDirectory();
	std::wstring dir = name + L"\\tessdata";

	assert(api->Init(utility::to_string(dir).c_str(), "jpn") == 0);

	api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
}

Uma::~Uma()
{
	if (thread) {
		Stop();
	}
	delete api;
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

void Uma::Debug()
{
	Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromFile(TEXT("debug2.png"));
	cv::Mat mat = BitmapToCvMat(image);

	//GetCharaEventText(mat);
	GetCardEventText(mat);

	delete image;
}

cv::Mat Uma::BitmapToCvMat(Gdiplus::Bitmap* image)
{
	Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());
	Gdiplus::BitmapData data;
	image->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat24bppRGB, &data);

	cv::Mat mat = cv::Mat(image->GetHeight(), image->GetWidth(), CV_8UC3, data.Scan0, data.Stride);

	image->UnlockBits(&data);

	return mat;
}

cv::Mat Uma::ImageBinarization(const cv::Mat& srcImg)
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
			
			cv::Mat srcImage = BitmapToCvMat(image);
			
			std::wstring EventName = GetCharaEventText(srcImage);
			_RPTW1(_CRT_WARN, L"ƒCƒxƒ“ƒg–¼: %ls\n", EventName.c_str());

			delete image;
		}

		Sleep(100);
	}
}

bool Uma::IsCharaEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(150, 100, 251), cv::Scalar(209, 198, 255), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

std::wstring Uma::GetCharaEventText(const cv::Mat& srcImg)
{
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::CharaEventBound.x * srcImg.size().width,
		Uma::CharaEventBound.y * srcImg.size().height,
		Uma::CharaEventBound.width * srcImg.size().width,
		Uma::CharaEventBound.height * srcImg.size().height
	));

	if (IsCharaEvent(cut)) {
		cv::Mat rsImg;

		cv::resize(cut, rsImg, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);
		cv::Mat bin = Uma::ImageBinarization(rsImg);

		int c = cv::countNonZero(bin);

		api->SetImage(bin.data, bin.size().width, bin.size().height, bin.channels(), bin.step1());
		api->Recognize(NULL);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
		std::wstring text = convert.from_bytes(api->GetUTF8Text());
		text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

		return text;
	}

	return L"";
}

bool Uma::IsCardEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(240, 145, 40), cv::Scalar(255, 210, 120), bg);
	cv::imwrite("test.png", bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

std::wstring Uma::GetCardEventText(const cv::Mat& srcImg)
{
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::CardEventBound.x * srcImg.size().width,
		Uma::CardEventBound.y * srcImg.size().height,
		Uma::CardEventBound.width * srcImg.size().width,
		Uma::CardEventBound.height * srcImg.size().height
	));

	if (IsCardEvent(cut)) {
		cv::Mat rsImg;

		cv::resize(cut, rsImg, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);
		cv::Mat bin = Uma::ImageBinarization(rsImg);

		int c = cv::countNonZero(bin);

		api->SetImage(bin.data, bin.size().width, bin.size().height, bin.channels(), bin.step1());
		api->Recognize(NULL);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
		std::wstring text = convert.from_bytes(api->GetUTF8Text());
		text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

		return text;
	}

	return L"";
}
