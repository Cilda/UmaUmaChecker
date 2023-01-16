#pragma once

#include <string>

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
	bool EnableDebug;
	bool SaveMissingEvent;
	std::wstring ScreenshotSavePath;
	std::wstring FontName;
	int FontSize;
	bool IsHideNoneChoise;
	bool IsShowStatusBar;
	int OptionMaxLine;
	int ImageType;
	bool EnableCheckUpdate;
	int OcrPoolSize;
	int Theme;
	int CaptureMode;
};

