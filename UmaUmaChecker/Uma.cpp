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
#include <wx/msw/msvcrt.h>

#include "simstring/simstring.h"

#include "utility.h"

#ifdef USE_MS_OCR
#include "../UmaOCRWrapper/UmaOCRWrapper.h"
#endif

const cv::Rect2d Uma::CharaEventBound = { 0.15206185567010309278350515463918, 0.18847603661820140010770059235326, 0.61094941634241245136186770428016, 0.02898550724637681159420289855072 };
const cv::Rect2d Uma::CardEventBound = { 0.15206185567010309278350515463918, 0.18847603661820140010770059235326, 0.61094941634241245136186770428016, 0.02898550724637681159420289855072 };
const cv::Rect2d Uma::BottomChoiseBound = { 0.1038, 0.6286, 0.8415, 0.04047 };
const cv::Rect2d Uma::ScenarioChoiseBound = { 0.15206185567010309278350515463918, 0.18847603661820140010770059235326, 0.61094941634241245136186770428016, 0.02898550724637681159420289855072 };
const cv::Rect2d Uma::TrainingCharaSingleLineBound = { 0.3186, 0.1358, 0.66839, 0.02769 }; // { 0.3186, 0.1107, 0.4844, 0.05410 }
const cv::Rect2d Uma::TrainingCharaMultiLineBound = { 0.3186, 0.1107, 0.66839, 0.05410 }; // { 0.3186, 0.1107, 0.4844, 0.05410 }
const double Uma::ResizeRatio = 2.0;
const float Uma::UnsharpRatio = 2.0f;

Uma::Uma(wxFrame* frame)
{
	bDetected = false;
	bStop = false;
	thread = nullptr;
	CurrentCharacter = nullptr;
	CurrentEvent = nullptr;
	api = new tesseract::TessBaseAPI();
	apiMulti = new tesseract::TessBaseAPI();

	this->frame = frame;
}

Uma::~Uma()
{
	if (thread) {
		Stop();
	}
	delete api;
	delete apiMulti;
}

void Uma::Init()
{
	CurrentEvent = nullptr;
	CurrentCharacter = nullptr;

	if (SkillLib.Load()) {
	}

	api->Init(utility::to_string(utility::GetExeDirectory() + L"\\tessdata").c_str(), "jpn");
	api->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

	apiMulti->Init(utility::to_string(utility::GetExeDirectory() + L"\\tessdata").c_str(), "jpn");

#ifdef USE_OCR
	InitOCR();
#endif

	Collector.Load();
}

HWND Uma::GetUmaWindow()
{
	return FindWindow(TEXT("UnityWndClass"), TEXT("umamusume"));
}

Gdiplus::Bitmap* Uma::ScreenShot()
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
	CurrentEvent = nullptr;

	wxLogDebug(wxT("ストップ====================="));
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
	//cv::bitwise_not(gray, gray);
	//cv::threshold(gray, bin, 0, 255, cv::THRESH_OTSU);
	cv::threshold(gray, bin, 236, 255, cv::THRESH_BINARY_INV);

	//cv::Mat bin2;
	//cv::erode(bin, bin2, cv::Mat(2, 2, CV_8U, cv::Scalar(1)));

	return bin.clone();
}

void Uma::MonitorThread()
{
	while (!bStop) {
		auto start = std::chrono::system_clock::now();

		Gdiplus::Bitmap* image = ScreenShot();
		if (image) {
			cv::Mat srcImage = BitmapToCvMat(image);
			bool bScaned = false;

			auto start1 = std::chrono::system_clock::now();

			EventSource* event = DetectEvent(srcImage, &bScaned);
			if (event) {
				if (event != CurrentEvent) {
					CurrentEvent = event;

					HBITMAP hBmp;

					image->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBmp);

					wxThreadEvent event(wxEVT_THREAD);
					event.SetId(1);
					event.SetPayload(hBmp);
					wxQueueEvent(frame, event.Clone());
				}
			}

			if (PrevEventHash != 0 && PrevEventHash != EventHash) {
				EventHash = PrevEventHash;
				Config* config = Config::GetInstance();
				if (config->SaveMissingEvent) {
					std::wstring directory = config->ScreenshotSavePath + L"\\";
					if (config->ScreenshotSavePath.empty()) {
						directory = utility::GetExeDirectory() + L"\\screenshots\\";
						CreateDirectoryW(directory.c_str(), NULL);
						directory += L"AutoSave\\";
						CreateDirectoryW(directory.c_str(), NULL);
					}

					std::wstring savename = directory
						+ std::wstring(L"debug_")
						+ std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
						+ L".png";

					CLSID clsid;
					utility::GetEncoderClsid(L"image/png", &clsid);
					image->Save(savename.c_str(), &clsid);
				}
			}

			auto end1 = std::chrono::system_clock::now();
			auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();

			wxLogDebug(wxT("DetectEvent(): %lld msec"), msec1);

			if (!bScaned) {
				auto start2 = std::chrono::system_clock::now();

				EventRoot* root = DetectTrainingCharaName(srcImage);
				if (root) {
					wxThreadEvent event(wxEVT_THREAD);
					event.SetId(2);
					event.SetString(root->Name);
					wxQueueEvent(frame, event.Clone());
				}

				auto end2 = std::chrono::system_clock::now();
				auto msec2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();

				wxLogDebug(wxT("DetectTrainingCharaName(): %lld msec"), msec2);
			}

			delete image;
		}

		auto end = std::chrono::system_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		wxLogDebug(wxT("MonitorThread() ループ処理時間: %lld msec"), msec);

		if (msec < 1000) Sleep(1000 - msec);
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
	//cv::pyrUp(cut, rsImg, cv::Size(rsImg.cols * ResizeRatio, rsImg.rows * ResizeRatio));
	UnsharpMask(rsImg, rsImg, UnsharpRatio);

	if (IsCharaEvent(rsImg)) {
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::bitwise_not(gray, gray);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		cv::Mat blur;

		//cv::resize(bin, bin, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);

		RemoveWhiteSpace(bin, bin);
		cv::dilate(bin, blur, cv::Mat(2, 2, CV_8U, cv::Scalar(1)), cv::Point(-1, -1), 1);

		std::vector<std::wstring> text_list;
		{
			auto a1 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin); });
			auto a2 = std::async(std::launch::async, [&] { AsyncFunction(text_list, gray); });
			auto a3 = std::async(std::launch::async, [&] { AsyncFunction(text_list, blur); });
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

std::wstring Uma::GetTextFromImage(const cv::Mat& img)
{
	auto start = std::chrono::system_clock::now();

	std::lock_guard<std::mutex> lock(mutex);

	api->SetImage(img.data, img.size().width, img.size().height, img.channels(), img.step1());
	api->Recognize(NULL);

	const std::unique_ptr<const char[]> utf8_text(api->GetUTF8Text());
	std::wstring text = utility::ConvertUtf8ToUtf16(utf8_text.get());
	text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

	//Collector.Collect(text);

	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	//wxLogDebug(wxT("GetTextFromImage(): %lld"), msec);

	return text;
}

std::wstring Uma::GetMultiTextFromImage(cv::Mat& img)
{
	auto start = std::chrono::system_clock::now();

	std::lock_guard<std::mutex> lock(mutex);

	apiMulti->SetImage(img.data, img.size().width, img.size().height, img.channels(), img.step1());
	apiMulti->Recognize(NULL);

	const std::unique_ptr<const char[]> utf8_text(apiMulti->GetUTF8Text());
	std::wstring text = utility::ConvertUtf8ToUtf16(utf8_text.get());
	text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

	//Collector.Collect(text);

	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	//wxLogDebug(wxT("GetMultiTextFromImage(): %lld"), msec);

	return text;
}

void Uma::AsyncFunction(std::vector<std::wstring>& strs, const cv::Mat& img)
{
	std::wstring str = GetTextFromImage(img);
	if (str.empty()) return;

	strs.push_back(str);
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

double Uma::CalcTextMatchRate(const std::wstring& stext, const std::wstring& dtext)
{
	simstring::ngram_generator gen(1, false);

	double max_rate = 0.0;
	std::vector<std::wstring> basengrams;

	gen(stext, std::inserter(basengrams, basengrams.end()));


	std::vector<std::wstring> ngrams;
	int total = 0, equal = 0;

	gen(dtext, std::inserter(ngrams, ngrams.end()));

	for (auto& word1 : basengrams) {
		for (auto& word2 : ngrams) {
			if (word1 == word2) {
				equal++;
			}
		}

		total++;
	}

	return (double)equal / total;
}

void Uma::RemoveWhiteSpace(const cv::Mat& mat, cv::Mat& output)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Point ret;

	cv::findContours(mat, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < contours.size() - 1; i++) {
		for (auto& pt : contours[i]) {
			if (pt.x > ret.x) ret.x = pt.x;
			if (pt.y > ret.y) ret.y = pt.y;
		}
	}

	if (ret.x + 10 > mat.size().width) output = mat;
	else output = cv::Mat(mat, cv::Rect(0, 0, ret.x + 10, mat.size().height));
}

void Uma::UnsharpMask(const cv::Mat& mat, const cv::Mat& dst, float k)
{
	float kernelData[] = {
		-k / 9.0f, -k / 9.0f, -k / 9.0f,
		-k / 9.0f, 1 + (8 * k) / 9.0f, -k / 9.0f,
		-k / 9.0f, -k / 9.0f, -k / 9.0f
	};

	cv::Mat kernel(3, 3, CV_32F, kernelData);
	cv::filter2D(mat, dst, -1, kernel);
}

size_t Uma::CreateHash(const std::vector<std::wstring>& strs)
{
	size_t hash = 0;

	for (auto& str : strs) {
		int index = 0;
		for (wchar_t c : str) {
			hash |= c << (index % (sizeof(hash) * 8 - 8));
			index++;
		}
	}

	return hash;
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
	if (!text.empty()) {
		auto event = SkillLib.RetrieveEventFromOptionTitle(text);
		if (event) return event;
	}

	text = GetTextFromImage(gray);
	if (!text.empty()) {
		auto event = SkillLib.RetrieveEventFromOptionTitle(text);
		if (event) return event;
	}

	return nullptr;
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
	if (!text.empty()) {
		auto event = SkillLib.RetrieveCharaEventFromOptionTitle(text);
		if (event) return event;
	}

	text = GetTextFromImage(gray);
	if (!text.empty()) {
		auto event = SkillLib.RetrieveCharaEventFromOptionTitle(text);
		if (event) return event;
	}

	return nullptr;
}

EventSource* Uma::DetectEvent(const cv::Mat& srcImg, bool* bScaned)
{
	if (bScaned) *bScaned = false;

	// サポートカードイベント
	std::vector<std::wstring> events = RecognizeCardEventText(srcImg);
	if (!events.empty()) {
		if (bScaned) *bScaned = true;
		auto event = GetCardEvent(events);
		if (!event) event = GetEventByBottomOption(srcImg);
		if (event) {
			return event.get();
		}
		PrevEventHash = CreateHash(events);
		return nullptr;
	}

	if (CurrentCharacter) {
		events = RecognizeCharaEventText(srcImg);
		if (!events.empty()) {
			if (bScaned) *bScaned = true;
			auto event = GetCharaEvent(events);
			if (!event) event = GetCharaEventByBottomOption(srcImg);
			if (event) {
				return event.get();
			}
			PrevEventHash = CreateHash(events);
			return nullptr;
		}
	}

	events = RecognizeScenarioEventText(srcImg);
	if (!events.empty()) {
		if (bScaned) *bScaned = true;
		auto event = GetScenarioEvent(events);
		if (event) {
			return event.get();
		}
		PrevEventHash = CreateHash(events);
		return nullptr;
	}

	return nullptr;
}

EventRoot* Uma::DetectTrainingCharaName(const cv::Mat& srcImg)
{
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::TrainingCharaMultiLineBound.x * srcImg.size().width,
		Uma::TrainingCharaMultiLineBound.y * srcImg.size().height,
		Uma::TrainingCharaMultiLineBound.width * srcImg.size().width,
		Uma::TrainingCharaMultiLineBound.height * srcImg.size().height
	));
	cv::Mat rsImg, gray, bin, bg;
	cv::Mat cloneImg;

	// 判定
	cv::inRange(cut, cv::Scalar(15, 55, 115), cv::Scalar(80, 114, 160), bg);
	double ratio = (double)cv::countNonZero(bg) / bg.size().area();

	if (ratio > 0.05) {
		cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::threshold(gray, bin, 85, 255, cv::THRESH_BINARY_INV);

		std::wstring text = GetMultiTextFromImage(bin);

		auto chara = SkillLib.RetrieveCharaName(text);
		if (chara.get() == CurrentCharacter) return nullptr;

		return chara ? chara.get() : nullptr;
	}

	return nullptr;
}

std::shared_ptr<EventSource> Uma::GetCardEvent(const std::vector<std::wstring>& text_list)
{
	double best_rate = 0.0;
	std::shared_ptr<EventSource> rvalue;

	for (auto& text : text_list) {
		auto ret = SkillLib.RetrieveEvent(text);
		if (ret) {
			double rate = CalcTextMatchRate(text, ret->Name);
			if (rate > best_rate) {
				best_rate = rate;
				rvalue = ret;
			}
		}
	}

	return best_rate > 0.0 ? rvalue : nullptr;
}

std::shared_ptr<EventSource> Uma::GetCharaEvent(const std::vector<std::wstring>& text_list)
{
	if (!CurrentCharacter) return nullptr;

	double best_rate = 0.0;
	std::shared_ptr<EventSource> rvalue;

	for (auto& text : text_list) {
		auto ret = SkillLib.RetrieveCharaEvent(text, CurrentCharacter->Name);
		if (ret) {
			double rate = CalcTextMatchRate(text, ret->Name);
			if (rate > best_rate) {
				best_rate = rate;
				rvalue = ret;
			}
		}
	}

	return best_rate > 0.0 ? rvalue : nullptr;
}

std::shared_ptr<EventSource> Uma::GetScenarioEvent(const std::vector<std::wstring>& text_list)
{
	double best_rate = 0.0;
	std::shared_ptr<EventSource> rvalue;

	for (auto& text : text_list) {
		auto ret = SkillLib.RetrieveScenarioEvent(text);
		if (ret) {
			double rate = CalcTextMatchRate(text, ret->Name);
			if (rate > best_rate) {
				best_rate = rate;
				rvalue = ret;
			}
		}
	}

	return best_rate > 0.0 ? rvalue : nullptr;
}

bool Uma::Reload()
{
	SkillLib.Clear();

	SkillLib.Load();

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
	//cv::pyrUp(cut, rsImg, cv::Size(rsImg.cols * ResizeRatio, rsImg.rows * ResizeRatio));
	UnsharpMask(rsImg, rsImg, UnsharpRatio);

	if (IsCardEvent(cut)) {
		cv::Mat gray, blur;

		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::bitwise_not(gray, gray);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		std::wstring text = GetTextFromImage(bin);

		//cv::resize(bin, bin, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);

		RemoveWhiteSpace(bin, bin);
		cv::dilate(bin, blur, cv::Mat(2, 2, CV_8U, cv::Scalar(1)), cv::Point(-1, -1), 1);

		std::vector<std::wstring> text_list;
		{
			auto a1 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin); });
			auto a2 = std::async(std::launch::async, [&] { AsyncFunction(text_list, gray); });
			auto a3 = std::async(std::launch::async, [&] { AsyncFunction(text_list, blur); });
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
	//cv::pyrUp(cut, rsImg, cv::Size(rsImg.cols * ResizeRatio, rsImg.rows * ResizeRatio));
	UnsharpMask(rsImg, rsImg, UnsharpRatio);

	if (IsScenarioEvent(cut)) {
		cv::Mat gray;
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::bitwise_not(gray, gray);
		cv::Mat bin = Uma::ImageBinarization(rsImg);
		std::wstring text = GetTextFromImage(bin);
		cv::Mat blur;

		//cv::resize(bin, bin, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);

		RemoveWhiteSpace(bin, bin);
		cv::dilate(bin, blur, cv::Mat(2, 2, CV_8U, cv::Scalar(1)), cv::Point(-1, -1), 1);

		std::vector<std::wstring> text_list;
		{
			auto a1 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin); });
			auto a2 = std::async(std::launch::async, [&] { AsyncFunction(text_list, gray); });
			auto a3 = std::async(std::launch::async, [&] { AsyncFunction(text_list, blur); });
		}

		AppendCollectedText(text_list);

		return text_list;
	}

	return std::vector<std::wstring>();
}
