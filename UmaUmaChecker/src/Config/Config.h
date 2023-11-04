#pragma once

#include <string>
#include "Capture/CaptureMode.h"

class Config
{
public:
	Config();
	virtual ~Config();

	bool Load();
	void Save();

	std::wstring GetImageExtension();
	std::wstring GetImageMimeType();

public:
	static Config* GetInstance();

public:
	int WindowX;
	int WindowY;
	int WindowWidth;
	bool EnableDebug;
	bool SaveMissingEvent;
	std::wstring ScreenshotSavePath;
	std::wstring FontName;
	std::wstring Language;
	int FontSize;
	bool IsHideNoneChoise;
	bool IsShowStatusBar;
	int OptionMaxLine;
	int ImageType;
	bool EnableCheckUpdate;
	int OcrPoolSize;
	int Theme;
	CaptureMode CaptureMode;
};

