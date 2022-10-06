#pragma once

#include "EventLibrary.h"
#include "TextCollector.h"

#include <wx/frame.h>

#include <objbase.h>
//#include <minmax.h>
#include <gdiplus.h>
#include <thread>
#include <opencv2/opencv.hpp>
#include <mutex>

#ifndef USE_MS_OCR
#include <tesseract/baseapi.h>
#endif

#include "Config.h"


#define WM_CHANGEUMAEVENT (WM_USER + 1)

class Uma
{
public:
	Uma(wxFrame* frame);
	~Uma();

	void Init();

	HWND GetUmaWindow();
	Gdiplus::Bitmap* ScreenShot();

	bool Start();
	void Stop();

	void SetNotifyTarget(HWND hWnd);
	void SetTrainingCharacter(const std::wstring& CharaName);

	const std::vector<std::shared_ptr<EventRoot>>& GetCharacters() const {
		return SkillLib.GetCharacters();
	}

	EventSource* DetectEvent(const cv::Mat& srcImg);

	bool Reload();
	bool IsStarted() const { return thread != nullptr; }

public:
	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	static cv::Mat ImageBinarization(cv::Mat& srcImg);

private:
	void MonitorThread();

	// イベント名認識
	std::vector<std::wstring> RecognizeCharaEventText(const cv::Mat& srcImg);
	std::vector<std::wstring> RecognizeCardEventText(const cv::Mat& srcImg);
	std::vector<std::wstring> RecognizeScenarioEventText(const cv::Mat& srcImg);

	// イベント取得
	std::shared_ptr<EventSource> GetCardEvent(const std::vector<std::wstring>& text_list);
	std::shared_ptr<EventSource> GetCharaEvent(const std::vector<std::wstring>& text_list);
	std::shared_ptr<EventSource> GetScenarioEvent(const std::vector<std::wstring>& text_list);
	std::shared_ptr<EventSource> GetEventByBottomOption(const cv::Mat& srcImg);
	std::shared_ptr<EventSource> GetCharaEventByBottomOption(const cv::Mat& srcImg);

	bool IsCharaEvent(const cv::Mat& srcImg);
	bool IsCardEvent(const cv::Mat& srcImg);
	bool IsScenarioEvent(const cv::Mat& srcImg);

	std::wstring GetTextFromImage(cv::Mat& img);
	void AppendCollectedText(std::vector<std::wstring>& text_list);

public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界
	static const cv::Rect2d CardEventBound;
	static const cv::Rect2d BottomChoiseBound;
	static const cv::Rect2d ScenarioChoiseBound;

public:
	std::wstring EventName;
	EventSource* CurrentEvent;
	Config config;

private:
	bool bDetected;
	bool bStop;
	std::thread* thread;
	HWND hTargetWnd;
	EventLibrary SkillLib;
	TextCollector Collector;
	EventRoot* CurrentCharacter;
	tesseract::TessBaseAPI* api;
	std::wstring DetectedEventName;
	std::mutex mutex;

	wxFrame* frame;

private:
	static const double ResizeRatio;
};