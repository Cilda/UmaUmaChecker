#pragma once

#include "EventLibrary.h"

#include <thread>
#include <opencv2/opencv.hpp>

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

public:
	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
	static cv::Mat ImageBinarization(const cv::Mat& srcImg);

private:
	void MonitorThread();

	bool IsCharaEvent(const cv::Mat& srcImg);
	std::wstring GetCharaEventText(const cv::Mat& srcImg);

	bool IsCardEvent(const cv::Mat& srcImg);
	std::wstring GetCardEventText(const cv::Mat& srcImg);

public:
	static const cv::Rect2d CharaEventBound; // キャライベント境界
	static const cv::Rect2d CardEventBound;

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
	Character* CurrentCharacter;

	std::wstring DetectedEventName;

private:
	static const double ResizeRatio;
};