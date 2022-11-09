#pragma once

#include "EventLibrary.h"
#include "TextCollector.h"

#include <wx/frame.h>

#include <objbase.h>
#include <gdiplus.h>
#include <thread>
#include <mutex>

#include <opencv2/opencv.hpp>


#ifndef USE_MS_OCR
#include <tesseract/baseapi.h>
#endif

#include "Config.h"


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

	void SetTrainingCharacter(const std::wstring& CharaName);

	const std::vector<std::vector<std::shared_ptr<EventRoot>>>& GetCharacters() const {
		return SkillLib.GetCharacters();
	}

	EventSource* DetectEvent(const cv::Mat& srcImg, bool* bScaned = nullptr);
	EventRoot* DetectTrainingCharaName(const cv::Mat& srcImg);

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

	// イベント判定
	bool IsCharaEvent(const cv::Mat& srcImg);
	bool IsCardEvent(const cv::Mat& srcImg);
	bool IsScenarioEvent(const cv::Mat& srcImg);

	std::wstring GetTextFromImage(const cv::Mat& img);
	std::wstring GetMultiTextFromImage(cv::Mat& img);

	void AsyncFunction(std::vector<std::wstring>& strs, const cv::Mat& img);

	void AppendCollectedText(std::vector<std::wstring>& text_list);
	double CalcTextMatchRate(const std::wstring& stext, const std::wstring& dtext);
	void RemoveWhiteSpace(const cv::Mat& mat, cv::Mat& output);
	void UnsharpMask(const cv::Mat& mat, const cv::Mat& dst, float k);

	// 文字配列からハッシュ取得
	size_t CreateHash(const std::vector<std::wstring>& strs);
public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界
	static const cv::Rect2d CardEventBound;
	static const cv::Rect2d BottomChoiseBound;
	static const cv::Rect2d ScenarioChoiseBound;
	static const cv::Rect2d TrainingCharaSingleLineBound;
	static const cv::Rect2d TrainingCharaMultiLineBound;

public:
	EventSource* CurrentEvent;

private:
	bool bDetected;
	bool bStop;
	std::thread* thread;
	EventLibrary SkillLib;
	TextCollector Collector;
	EventRoot* CurrentCharacter;
	tesseract::TessBaseAPI* api;
	tesseract::TessBaseAPI* apiMulti;
	std::wstring DetectedEventName;
	std::mutex mutex;

	size_t EventHash = 0;
	size_t PrevEventHash = 0;

	// for event
	wxFrame* frame;

private:
	static const double ResizeRatio;
	static const float UnsharpRatio;
};