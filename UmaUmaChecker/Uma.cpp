#include <winsock2.h>
#include <Windows.h>
#include <WinInet.h>

//

//#include <gdiplus.h>

#include "Uma.h"

#include <codecvt>
#include <crtdbg.h>
#include <future>
#include "utility.h"

#include <wx/log.h>
#include <wx/utils.h>

#ifdef USE_MS_OCR
#include "../UmaOCRWrapper/UmaOCRWrapper.h"
#endif

const cv::Rect2d Uma::CharaEventBound = { 0.1532, 0.1876, 0.6118, 0.03230 };
const cv::Rect2d Uma::CardEventBound = { 0.1532, 0.1876, 0.6118, 0.03230 };
const cv::Rect2d Uma::BottomChoiseBound = { 0.1038, 0.6286, 0.8415, 0.04047 };
const cv::Rect2d Uma::ScenarioChoiseBound = { 0.1636, 0.1929, 0.6051, 0.02485 };
const double Uma::ResizeRatio = 2.0;

Uma::Uma(wxFrame* frame)
{
	bDetected = false;
	bStop = false;
	thread = nullptr;
	hTargetWnd = NULL;
	CurrentCharacter = nullptr;
	CurrentEvent = nullptr;
	api = new tesseract::TessBaseAPI();

	this->frame = frame;
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
	CurrentEvent = nullptr;
	CurrentCharacter = nullptr;

	if (SkillLib.Load()) {
		SkillLib.InitEventDB();
		SkillLib.InitCharaDB();
		SkillLib.InitScenarioEventDB();
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

	wxLogDebug(wxT("スタート====================="));

	return true;
}

void Uma::Stop()
{
	if (!thread) return;

	bStop = true;
	thread->join();
	delete thread;
	thread = nullptr;

	wxLogDebug(wxT("ストップ====================="));
}

void Uma::SetNotifyTarget(HWND hWnd)
{
	hTargetWnd = hWnd;
}

void Uma::SetTrainingCharacter(const std::wstring& CharaName)
{
	CurrentCharacter = SkillLib.GetCharacter(CharaName);
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

cv::Mat Uma::ImageBinarization(cv::Mat& srcImg)
{
	cv::Mat gray;
	cv::Mat bin;

	/*
	cv::inRange(gray, cv::Scalar(242, 242, 242), cv::Scalar(255, 255, 255), bin);
	cv::bitwise_not(bin, bin);
	*/
	cv::cvtColor(srcImg, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 236, 255, cv::THRESH_BINARY_INV);

	return bin.clone();
}

void Uma::MonitorThread()
{
	while (!bStop) {
		auto start = std::chrono::system_clock::now();

		Gdiplus::Bitmap* image = ScreenShot();
		if (image) {
			cv::Mat srcImage = BitmapToCvMat(image);
			
			CurrentEvent = DetectEvent(srcImage);
			if (CurrentEvent) {
				if (EventName != CurrentEvent->Name) {
					EventName = CurrentEvent->Name;

					wxThreadEvent event(wxEVT_THREAD);
					wxQueueEvent(frame, event.Clone());
				}
			}

			delete image;
		}

		auto end = std::chrono::system_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		//wxLogDebug(wxT("MonitorThread() ループ処理時間: %lld msec"), msec);

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

std::vector<std::wstring> Uma::RecognizeCharaEventText(const cv::Mat& srcImg)
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
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		cv::Mat blur;

		cv::medianBlur(bin, blur, 5);

		std::vector<std::wstring> text_list;
		{
			auto a1 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(gray)); });
			auto a2 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(bin)); });
			auto a3 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(blur)); });
		}

		AppendCollectedText(text_list);

		return text_list;
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

bool Uma::IsScenarioEvent(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(20, 200, 110), cv::Scalar(80, 240, 170), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.3;
}

std::wstring Uma::GetTextFromImage(cv::Mat& img)
{
	auto start = std::chrono::system_clock::now();

	std::lock_guard<std::mutex> lock(mutex);

	api->SetImage(img.data, img.size().width, img.size().height, img.channels(), img.step1());
	api->Recognize(NULL);

	std::wstring text = utility::ConvertUtf8ToUtf16(api->GetUTF8Text());
	text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

	//Collector.Collect(text);

	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	wxLogDebug(wxT("GetTextFromImage(): %lld"), msec);

	return text;
}

void Uma::AppendCollectedText(std::vector<std::wstring>& text_list)
{
	std::vector<std::wstring> vecs;

	for (auto text : text_list) {
		if (Collector.Collect(text)) {
			vecs.push_back(text);
		}
	}

	text_list.insert(text_list.begin(), vecs.begin(), vecs.end());
}

std::shared_ptr<EventSource> Uma::GetEventByBottomOption(const cv::Mat& srcImg)
{
	cv::Mat rsImg, gray, bin;
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::BottomChoiseBound.x * srcImg.size().width,
		Uma::BottomChoiseBound.y * srcImg.size().height,
		Uma::BottomChoiseBound.width * srcImg.size().width,
		Uma::BottomChoiseBound.height * srcImg.size().height
	));

	cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
	cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 90, 255, cv::THRESH_BINARY);

	std::wstring text = GetTextFromImage(bin);
	return SkillLib.RetrieveEventFromOptionTitle(text);
}

std::shared_ptr<EventSource> Uma::GetCharaEventByBottomOption(const cv::Mat& srcImg)
{
	cv::Mat rsImg, gray, bin;
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::BottomChoiseBound.x * srcImg.size().width,
		Uma::BottomChoiseBound.y * srcImg.size().height,
		Uma::BottomChoiseBound.width * srcImg.size().width,
		Uma::BottomChoiseBound.height * srcImg.size().height
	));

	cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
	cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 90, 255, cv::THRESH_BINARY);

	std::wstring text = GetTextFromImage(bin);
	return SkillLib.RetrieveCharaEventFromOptionTitle(text);
}

EventSource* Uma::DetectEvent(const cv::Mat& srcImg)
{
	// サポートカードイベント
	std::vector<std::wstring> events = RecognizeCardEventText(srcImg);
	if (!events.empty()) {
		auto event = GetCardEvent(events);
		if (!event) event = GetEventByBottomOption(srcImg);
		if (event) {
			return event.get();
		}
	}

	if (CurrentCharacter) {
		events = RecognizeCharaEventText(srcImg);
		if (!events.empty()) {
			auto event = GetCharaEvent(events);
			if (!event) event = GetCharaEventByBottomOption(srcImg);
			if (event) {
				return event.get();
			}
		}
	}

	events = RecognizeScenarioEventText(srcImg);
	if (!events.empty()) {
		auto event = GetScenarioEvent(events);
		if (event) {
			return event.get();
		}
	}



	return nullptr;
}

std::shared_ptr<EventSource> Uma::GetCardEvent(const std::vector<std::wstring>& text_list)
{
	for (auto& text : text_list) {
		auto ret = SkillLib.RetrieveEvent(text);
		if (ret) return ret;
	}

	return nullptr;
}

std::shared_ptr<EventSource> Uma::GetCharaEvent(const std::vector<std::wstring>& text_list)
{
	if (!CurrentCharacter) return nullptr;

	for (auto& text : text_list) {
		auto ret = SkillLib.RetrieveCharaEvent(text, CurrentCharacter->Name);
		if (ret) return ret;
	}

	return nullptr;
}

std::shared_ptr<EventSource> Uma::GetScenarioEvent(const std::vector<std::wstring>& text_list)
{
	for (auto& text : text_list) {
		auto ret = SkillLib.RetrieveScenarioEvent(text);
		if (ret) return ret;
	}

	return nullptr;
}

bool Uma::Reload()
{
	SkillLib.Clear();

	if (SkillLib.Load()) {
		SkillLib.InitEventDB();
		SkillLib.InitCharaDB();
		SkillLib.InitScenarioEventDB();
	}

	return true;
}

std::vector<std::wstring> Uma::RecognizeCardEventText(const cv::Mat& srcImg)
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
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		std::wstring text = GetTextFromImage(bin);
		cv::Mat blur;

		cv::medianBlur(bin, blur, 5);

		std::vector<std::wstring> text_list;
		{
			auto a1 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(gray)); });
			auto a2 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(bin)); });
			auto a3 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(blur)); });
		}

		AppendCollectedText(text_list);

		return text_list;
	}

	return std::vector<std::wstring>();
}

std::vector<std::wstring> Uma::RecognizeScenarioEventText(const cv::Mat& srcImg)
{
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::ScenarioChoiseBound.x * srcImg.size().width,
		Uma::ScenarioChoiseBound.y * srcImg.size().height,
		Uma::ScenarioChoiseBound.width * srcImg.size().width,
		Uma::ScenarioChoiseBound.height * srcImg.size().height
	));
	cv::Mat rsImg;

	cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);

	if (IsScenarioEvent(cut)) {
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		std::wstring text = GetTextFromImage(bin);
		cv::Mat blur;

		cv::medianBlur(bin, blur, 5);

		std::vector<std::wstring> text_list;
		{
			auto a1 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(gray)); });
			auto a2 = std::async(std::launch::async, [&] { text_list.push_back(GetTextFromImage(blur)); });
		}

		AppendCollectedText(text_list);

		return text_list;
	}

	return std::vector<std::wstring>();
}
