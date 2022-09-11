#include <Windows.h>
#include <gdiplus.h>

#include "Uma.h"


#include <codecvt>
#include <crtdbg.h>
#include "utility.h"


const cv::Rect2d Uma::CharaEventBound = { 0.1545, 0.1884, 0.6118, 0.03140 };
const cv::Rect2d Uma::CardEventBound = { 0.1532, 0.1876, 0.6118, 0.03030 };

Uma::Uma()
{
	bDetected = false;
	bStop = false;
	thread = nullptr;
	api = new tesseract::TessBaseAPI();
	hTargetWnd = NULL;
}

Uma::~Uma()
{
	if (thread) {
		Stop();
	}
	delete api;
}

void Uma::Init()
{
	std::wstring name = utility::GetExeDirectory();
	std::wstring dir = name + L"\\tessdata";

	assert(api->Init(utility::to_string(dir).c_str(), "jpn") == 0);
	api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);

	if (SkillLib.Load()) {
		SkillLib.InitEventDB();
		SkillLib.InitCharaDB();
	}

	config.Load();
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
	BITMAPINFO bmpinfo;
	byte* lpPixel;

	GetWindowRect(hWnd, &rw);
	GetClientRect(hWnd, &rc);

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

	SelectObject(hdc_mem, hBmp);	
	BitBlt(hdc_mem, 0, 0, rc.right, rc.bottom, hdc, pt.x, pt.y, SRCCOPY);

	Gdiplus::Bitmap* image = new Gdiplus::Bitmap(hBmp, NULL);

	DeleteDC(hdc_mem);
	DeleteObject(hBmp);
	ReleaseDC(hWnd, hdc);

	return image;
}

bool Uma::Start()
{
	if (thread) return false;

	bStop = false;
	thread = new std::thread(&Uma::MonitorThread, this);
	return true;
}

void Uma::Stop()
{
	if (!thread) return;

	bStop = true;
	thread->join();
	delete thread;
	thread = nullptr;
}

void Uma::SetNotifyTarget(HWND hWnd)
{
	hTargetWnd = hWnd;
}

cv::Mat Uma::BitmapToCvMat(Gdiplus::Bitmap* image)
{
	assert(image->GetPixelFormat() == PixelFormat24bppRGB);
	Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());
	Gdiplus::BitmapData data;
	image->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, image->GetPixelFormat(), &data);

	cv::Mat mat = cv::Mat(image->GetHeight(), image->GetWidth(), CV_8UC3, data.Scan0, data.Stride);

	image->UnlockBits(&data);

	return mat;
}

cv::Mat Uma::ImageBinarization(const cv::Mat& srcImg)
{
	cv::Mat gray;
	cv::Mat bin;

	cv::cvtColor(srcImg, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 0, 255, cv::THRESH_OTSU);

	return bin;
}

void Uma::MonitorThread()
{
	while (!bStop) {
		Gdiplus::Bitmap* image = ScreenShot();
		if (image) {
			cv::Mat srcImage = BitmapToCvMat(image);
			
			// サポートカードイベント
			std::wstring EventName = GetCardEventText(srcImage);
			if (!EventName.empty()) {
				if (this->EventName != EventName) {
					EventName = utility::replace(EventName, L":", L"：");
					EventName = utility::replace(EventName, L"\"", L"");

					if (SkillLib.EventMap.find(EventName) != SkillLib.EventMap.end()) {
						auto& skill = SkillLib.EventMap[EventName];

						CurrentEvent = &skill;
						this->EventName = EventName;
						if (hTargetWnd) PostMessage(hTargetWnd, WM_CHANGEUMAEVENT, 0, 0);
					}
					else {
						EventName = SkillLib.SearchEvent(EventName);

						if (!EventName.empty() && this->EventName != EventName) {
							if (SkillLib.EventMap.find(EventName) != SkillLib.EventMap.end()) {
								auto& skill = SkillLib.EventMap[EventName];

								CurrentEvent = &skill;
							}
							else {
								CurrentEvent = nullptr;
							}
						}
						else {
							CurrentEvent = nullptr;
						}

						if (hTargetWnd && this->EventName != EventName)
							PostMessage(hTargetWnd, WM_CHANGEUMAEVENT, 0, 0);

						this->EventName = EventName;
					}
				}
			}
			// キャライベント
			else {
				std::wstring EventName = GetCharaEventText(srcImage);
				if (!EventName.empty() && this->EventName != EventName) {
					//EventName = utility::replace(EventName, L":", L"：");
					//EventName = utility::replace(EventName, L"\"", L"");

					if (SkillLib.CharaMap.find(EventName) != SkillLib.CharaMap.end()) {
						auto& skill = SkillLib.CharaMap[EventName];

						CurrentEvent = &skill;
						this->EventName = EventName;
						if (hTargetWnd) PostMessage(hTargetWnd, WM_CHANGEUMAEVENT, 0, 0);
					}
					else {
						EventName = SkillLib.SearchCharaEvent(EventName);

						if (!EventName.empty() && this->EventName != EventName) {
							if (SkillLib.CharaMap.find(EventName) != SkillLib.CharaMap.end()) {
								auto& skill = SkillLib.CharaMap[EventName];

								CurrentEvent = &skill;
							}
							else {
								CurrentEvent = nullptr;
							}
						}
						else {
							CurrentEvent = nullptr;
						}

						if (hTargetWnd && this->EventName != EventName)
							PostMessage(hTargetWnd, WM_CHANGEUMAEVENT, 0, 0);

						this->EventName = EventName;
					}
				}
			}
			
			

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
	cv::Mat rsImg;

	cv::resize(cut, rsImg, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);

	if (IsCharaEvent(rsImg)) {
		cv::Mat bin = Uma::ImageBinarization(rsImg);

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
	cv::Mat rsImg;

	cv::resize(cut, rsImg, cv::Size(), 3.0, 3.0, cv::INTER_CUBIC);

	if (IsCardEvent(cut)) {
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		cv::resize(bin, bin, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);

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
