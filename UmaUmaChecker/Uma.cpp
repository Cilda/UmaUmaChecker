#include <Windows.h>
#include <gdiplus.h>

#include "Uma.h"

#include <codecvt>
#include <crtdbg.h>
#include <future>
#include "utility.h"

#ifdef USE_MS_OCR
#include "../UmaOCRWrapper/UmaOCRWrapper.h"
#endif

const cv::Rect2d Uma::CharaEventBound = { 0.1532, 0.1876, 0.6118, 0.03230 };
const cv::Rect2d Uma::CardEventBound = { 0.1532, 0.1876, 0.6118, 0.03230 };
const double Uma::ResizeRatio = 2.0;

Uma::Uma()
{
	bDetected = false;
	bStop = false;
	thread = nullptr;
	hTargetWnd = NULL;
	CurrentCharacter = nullptr;
	api = new tesseract::TessBaseAPI();
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
	if (SkillLib.Load()) {
		SkillLib.InitEventDB();
		SkillLib.InitCharaDB();
	}

	api->Init(utility::to_string(utility::GetExeDirectory() + L"\\tessdata").c_str(), "jpn");
	api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);

#ifdef USE_OCR
	InitOCR();
#endif

	config.Load();
	Collector.Load();
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

void Uma::SetTrainingCharacter(const std::wstring& CharaName)
{
	auto itr = SkillLib.CharaMap.find(CharaName);
	if (itr != SkillLib.CharaMap.end()) {
		CurrentCharacter = itr->second.get();
	}
	else {
		CurrentCharacter = nullptr;
	}
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

	/*
	cv::inRange(gray, cv::Scalar(242, 242, 242), cv::Scalar(255, 255, 255), bin);
	cv::bitwise_not(bin, bin);
	*/
	cv::cvtColor(srcImg, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 240, 255, cv::THRESH_BINARY_INV);

	return bin.clone();
}

void Uma::MonitorThread()
{
	while (!bStop) {
		Gdiplus::Bitmap* image = ScreenShot();
		if (image) {
			cv::Mat srcImage = BitmapToCvMat(image);
			
			// サポートカードイベント
			std::vector<std::wstring> events = GetCardEventText(srcImage);
			if (!events.empty()) {
				std::wstring EventName = GetCardEventName(events);
				if (this->EventName != EventName) {
					std::wstring OrgEventName = EventName;

					// デバッグ
					if (config.SaveMissingEvent && EventName.empty() && DetectedEventName != OrgEventName) {
						DetectedEventName = OrgEventName;

						CreateDirectoryW((utility::GetExeDirectory() + L"\\Logs\\").c_str(), NULL);
						CreateDirectoryW((utility::GetExeDirectory() + L"\\Logs\\support\\").c_str(), NULL);

						std::wstring path =
							utility::GetExeDirectory() +
							std::wstring(L"\\Logs\\support\\screenshot_") +
							std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) +
							L".png";

						cv::imwrite(utility::to_string(path), srcImage);
					}

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
			// キャライベント
			else if (CurrentCharacter) {
				std::vector<std::wstring> events = GetCharaEventText(srcImage);
				if (!events.empty()) {
					std::wstring EventName = GetCharaEventName(events);
					if (this->EventName != EventName) {
						std::wstring OrgEventName = EventName;

						// デバッグ
						if (config.SaveMissingEvent && EventName.empty() && DetectedEventName != OrgEventName) {
							DetectedEventName = OrgEventName;

							CreateDirectoryW((utility::GetExeDirectory() + L"\\Logs\\").c_str(), NULL);
							CreateDirectoryW((utility::GetExeDirectory() + L"\\Logs\\chara\\").c_str(), NULL);

							std::wstring path =
								utility::GetExeDirectory() +
								std::wstring(L"\\Logs\\chara\\screenshot_") +
								std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) +
								L".png";

							cv::imwrite(utility::to_string(path), srcImage);
						}

						if (!EventName.empty() && this->EventName != EventName) {
							if (CurrentCharacter->Events.find(EventName) != CurrentCharacter->Events.end()) {
								auto& skill = CurrentCharacter->Events[EventName];

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
				else {
					/*
					if (hTargetWnd && this->EventName != EventName)
						PostMessage(hTargetWnd, WM_CHANGEUMAEVENT, 0, 0);
					*/

					this->EventName = EventName;
					CurrentEvent = nullptr;
				}
			}

			delete image;
		}

		Sleep(1000);
	}
}

bool Uma::IsCharaEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(150, 100, 251), cv::Scalar(209, 198, 255), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

std::vector<std::wstring> Uma::GetCharaEventText(const cv::Mat& srcImg)
{
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::CharaEventBound.x * srcImg.size().width,
		Uma::CharaEventBound.y * srcImg.size().height,
		Uma::CharaEventBound.width * srcImg.size().width,
		Uma::CharaEventBound.height * srcImg.size().height
	));
	cv::Mat rsImg;

	cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);

	if (IsCharaEvent(rsImg)) {
#ifdef USE_MS_OCR
		cv::Mat recognizeImage = rsImg.clone();

		std::wstring text;
		wchar_t wText[1024];
		if (RecognizeText(
			recognizeImage.size().width,
			recognizeImage.size().height,
			recognizeImage.data, recognizeImage.total() * recognizeImage.elemSize(),
			recognizeImage.step,
			wText, 1024
		)) {
			text = wText;
			text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

			return text;
		}

		cv::Mat bin = Uma::ImageBinarization(rsImg);
		recognizeImage = bin;

		if (RecognizeTextFromGrayImage(
			recognizeImage.size().width,
			recognizeImage.size().height,
			recognizeImage.data, recognizeImage.total() * recognizeImage.elemSize(),
			recognizeImage.step,
			wText, 1024
		)) {
			text = wText;
			text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

			return text;
		}
#else
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		std::wstring text = GetTextFromImage(bin);

		std::vector<std::wstring> text_list;
		{
			std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(gray)); });
			std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(bin)); });
		}

		return text_list;
#endif
	}

	return std::vector<std::wstring>();
}

bool Uma::IsCardEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(240, 145, 40), cv::Scalar(255, 210, 120), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

std::wstring Uma::GetTextFromImage(cv::Mat& img)
{
	std::lock_guard<std::mutex> lock(mutex);

	api->SetImage(img.data, img.size().width, img.size().height, img.channels(), img.step1());
	api->Recognize(NULL);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
	std::wstring text = convert.from_bytes(api->GetUTF8Text());
	text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

	Collector.Collect(text);

	return text;
}

std::wstring Uma::GetCardEventName(const std::vector<std::wstring>& text_list)
{
	for (auto& text : text_list) {
		std::wstring ret = SkillLib.SearchEvent(text);
		if (!ret.empty()) return ret;
	}

	return L"";
}

std::wstring Uma::GetCharaEventName(const std::vector<std::wstring>& text_list)
{
	for (auto& text : text_list) {
		std::wstring ret = SkillLib.SearchCharaEvent(text);
		if (!ret.empty()) return ret;
	}

	return L"";
}

std::vector<std::wstring> Uma::GetCardEventText(const cv::Mat& srcImg)
{
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::CardEventBound.x * srcImg.size().width,
		Uma::CardEventBound.y * srcImg.size().height,
		Uma::CardEventBound.width * srcImg.size().width,
		Uma::CardEventBound.height * srcImg.size().height
	));
	cv::Mat rsImg;

	cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);

	if (IsCardEvent(cut)) {
#ifdef USE_MS_OCR
		cv::Mat recognizeImage = rsImg.clone();
		
		std::wstring text;
		wchar_t wText[1024];
		if (RecognizeText(
			recognizeImage.size().width,
			recognizeImage.size().height,
			recognizeImage.data, recognizeImage.total() * recognizeImage.elemSize(),
			recognizeImage.step,
			wText, 1024
		)) {
			text = wText;
			text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

			return text;
		}

		cv::Mat bin = Uma::ImageBinarization(rsImg);
		recognizeImage = bin;

		if (RecognizeTextFromGrayImage(
			recognizeImage.size().width,
			recognizeImage.size().height,
			recognizeImage.data, recognizeImage.total() * recognizeImage.elemSize(),
			recognizeImage.step,
			wText, 1024
		)) {
			text = wText;
			text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

			return text;
		}
#else
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		std::wstring text = GetTextFromImage(bin);

		std::vector<std::wstring> text_list;
		{
			std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(gray)); });
			std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(bin)); });
		}

		return text_list;
#endif
	}

	return std::vector<std::wstring>();
}
