#pragma once

#include "EventLibrary.h"
#include "TextCollector.h"

#include <wx/frame.h>

#include <objbase.h>
#include <gdiplus.h>
#include <thread>
#include <mutex>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include "Config.h"
#include "object_pool.hpp"

class Uma
{
public:
	Uma(wxFrame* frame);
	~Uma();

	void Init();

	bool Start();
	void Stop();

	bool SetTrainingCharacter(const std::wstring& CharaName);
	std::wstring GetTrainingCharacter() const { return CurrentCharacter ? CurrentCharacter->Name : L""; }

	const std::vector<std::vector<std::shared_ptr<EventRoot>>>& GetCharacters() const {
		return EventLib.CharaEvent.GetRanks();
	}

	EventSource* DetectEvent(const cv::Mat& srcImg, uint64* pHash = nullptr, std::vector<std::wstring>* pEvents = nullptr, bool* bScaned = nullptr);
	EventRoot* DetectTrainingCharaName(const cv::Mat& srcImg);

	bool Reload();
	bool IsStarted() const { return thread != nullptr; }

public:
	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	static cv::Mat ImageBinarization(cv::Mat& srcImg);

private:
	void MonitorThread();

	void ProcessEventAndCharacter(Gdiplus::Bitmap* image, const cv::Mat& srcImage);
	void ProcessStatus(Gdiplus::Bitmap* image, const cv::Mat& srcImage);

	// イベント名認識
	std::vector<std::wstring> RecognizeCharaEventText(const cv::Mat& srcImg, uint64* pHash);
	std::vector<std::wstring> RecognizeCardEventText(const cv::Mat& srcImg, uint64* pHash);
	std::vector<std::wstring> RecognizeScenarioEventText(const cv::Mat& srcImg, uint64* pHash);

	// イベント取得
	std::shared_ptr<EventSource> GetCardEvent(const std::vector<std::wstring>& text_list);
	std::shared_ptr<EventSource> GetCharaEvent(const std::vector<std::wstring>& text_list);
	std::shared_ptr<EventSource> GetScenarioEvent(const std::vector<std::wstring>& text_list);

	std::shared_ptr<EventSource> RecognizeBottomOption(const cv::Mat& srcImg, BaseData* data, EventRoot* root = nullptr);
	std::shared_ptr<EventSource> GetEventByBottomOption(const cv::Mat& srcImg);
	std::shared_ptr<EventSource> GetCharaEventByBottomOption(const cv::Mat& srcImg);
	std::shared_ptr<EventSource> GetScenarioEventByBottomOption(const cv::Mat& srcImg);

	// イベント判定
	bool IsCharaEvent(const cv::Mat& srcImg);
	bool IsCardEvent(const cv::Mat& srcImg);
	bool IsScenarioEvent(const cv::Mat& srcImg);
	bool IsBottomOption(const cv::Mat& srcImg);

	// OCR関数
	std::wstring GetTextFromImage(const cv::Mat& img);
	int GetNumericFromImage(const cv::Mat& img);

	void AsyncFunction(std::vector<std::wstring>& strs, const cv::Mat& img);

	// イベント名補完
	void AppendCollectedText(std::vector<std::wstring>& text_list);
	void ChangeCollectedText(std::wstring& text);
	double CalcTextMatchRate(const std::wstring& stext, const std::wstring& dtext);

	// 画像操作
	void RemoveWhiteSpace(const cv::Mat& mat, cv::Mat& output);
	void UnsharpMask(const cv::Mat& mat, cv::Mat& dst, float k);
	void ResizeBest(cv::Mat& src, cv::Mat& dest, int height);
	bool IsEventIcon(const cv::Mat& img);

	// ハッシュ関数
	uint64 GetImageHash(const cv::Mat& img);
	int GetHashLength(uint64 hash1, uint64 hash2);

	void OutputMissingEventInfo(Gdiplus::Bitmap* image, uint64 hash, const std::vector<std::wstring>& events);

	// ステータス取得用
	bool DetectCharaStatus(const cv::Mat& src);

public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界
	static const cv::Rect2d CardEventBound;
	static const cv::Rect2d BottomChoiseBound;
	static const cv::Rect2d ScenarioChoiseBound;
	static const cv::Rect2d TrainingCharaAliasNameBound;
	static const cv::Rect2d TrainingCharaNameBound;
	static const cv::Rect2d EventIconBound;
	static const cv::Rect2d StatusBounds[5];

public:
	EventSource* CurrentEvent;

private:
	bool bDetected;
	bool bStop;
	std::thread* thread;
	//EventLibrary SkillLib;
	TextCollector Collector;
	EventRoot* CurrentCharacter;
	std::wstring DetectedEventName;
	std::mutex mutex;

	uint64 EventHash = 0;
	uint64 PrevEventHash = 0;

	// for event
	wxFrame* frame;

private:
	static const double ResizeRatio;
	static const float UnsharpRatio;
};