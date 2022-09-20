#pragma once



#include "EventLibrary.h"
#include "TextCollector.h"

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
	Uma();
	~Uma();

	void Init();

	HWND GetUmaWindow();
	Gdiplus::Bitmap* ScreenShot();

	bool Start();
	void Stop();

	void SetNotifyTarget(HWND hWnd);
	void SetTrainingCharacter(const std::wstring& CharaName);

	const std::vector<std::shared_ptr<Character>>& GetCharacters() const {
		return SkillLib.Charas;
	}

	std::vector<std::wstring> GetCharaEventText(const cv::Mat& srcImg);
	std::vector<std::wstring> GetCardEventText(const cv::Mat& srcImg);
	std::wstring GetCardEventName(const std::vector<std::wstring>& text_list);
	std::wstring GetCharaEventName(const std::vector<std::wstring>& text_list);

	bool UpdateLibrary();

public:
	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	static cv::Mat ImageBinarization(const cv::Mat& srcImg);

private:
	void MonitorThread();

	bool IsCharaEvent(const cv::Mat& srcImg);
	bool IsCardEvent(const cv::Mat& srcImg);

	std::wstring GetTextFromImage(cv::Mat& img);
	std::wstring GetBottomChoiseTitle(cv::Mat& srcImg);

	bool UpdateFile(const std::wstring& url, const std::wstring& path);

public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界
	static const cv::Rect2d CardEventBound;
	static const cv::Rect2d BottomChoiseBound;

public:
	std::wstring EventName;
	Character::Event* CurrentEvent;
	Config config;

private:
	bool bDetected;
	bool bStop;
	std::thread* thread;
	HWND hTargetWnd;
	EventLibrary SkillLib;
	TextCollector Collector;
	Character* CurrentCharacter;
	tesseract::TessBaseAPI* api;
	std::wstring DetectedEventName;
	std::mutex mutex;

private:
	static const double ResizeRatio;
};