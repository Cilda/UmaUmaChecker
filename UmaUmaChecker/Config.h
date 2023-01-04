#pragma once

#include <string>

class Config
{
public:
	Config();
	virtual ~Config();

	bool Load();
	void Create();
	void Save();

	std::wstring GetImageExtension();
	std::wstring GetImageMimeType();

public:
	static Config* GetInstance();

public:
	int WindowX = 0;
	int WindowY = 0;
	bool EnableDebug = false;
	bool SaveMissingEvent = false;
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
};

