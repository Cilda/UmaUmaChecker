#include "Config.h"

#include <Windows.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <filesystem>

#include "Utils/utility.h"

using json = nlohmann::json;

Config::Config()
{
}

Config::~Config()
{
}

bool Config::Load()
{
	std::ifstream input(utility::GetExeDirectory() + L"\\config.json");

	try {
		json config = json::parse(input, nullptr, false);
		if (config.is_discarded()) config = json(json::value_t::object);

		WindowX = config.value("WindowX", 0);
		WindowY = config.value("WindowY", 0);
		WindowWidth = config.value("WindowWidth", -1);
		if (WindowWidth == 0) WindowWidth = -1;
		EnableDebug = config.value("Debug", false);
		SaveMissingEvent = config.value("SaveMissingEventName", false);
		ScreenshotSavePath = utility::from_u8string(config.value("ScreenshotSavePath", ""));
		FontName = utility::from_u8string(config.value("FontName", "Yu Gothic UI"));
		FontSize = config.value("FontSize", 9);
		IsHideNoneChoise = config.value("IsHideNoneChoise", false);
		IsShowStatusBar = config.value("IsShowStatusBar", false);
		OptionMaxLine = config.value("OptionMaxLine", 4);
		if (OptionMaxLine < 2) OptionMaxLine = 2;
		else if (OptionMaxLine > 10) OptionMaxLine = 10;
		ImageType = config.value("ImageType", 0);
		EnableCheckUpdate = config.value("EnableCheckUpdate", true);
		OcrPoolSize = config.value("OcrPoolSize", 2);
		Theme = config.value("Theme", 0);
		CaptureMode = (::CaptureMode)config.value("CaptureMode", 0);
		Language = utility::from_u8string(config.value("Language", "System"));
		UpdateUrl = utility::from_u8string(config.value("UpdateUrl", "https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/"));
		if (UpdateUrl.back() != '/') UpdateUrl += L"/";
		TesseractLanguage = utility::from_u8string(config.value("TesseractLanguage", "jpn"));
	}
	catch (json::exception& ex) {
		return false;
	}

	if (!input.good()) Save();

	if (ScreenshotSavePath.empty()) {
		std::wstring directory = utility::GetExeDirectory() + L"\\screenshots\\";
		CreateDirectoryW(directory.c_str(), NULL);
	}

	return true;
}

void Config::Save()
{
	json config;

	config["WindowX"] = WindowX;
	config["WindowY"] = WindowY;
	config["WindowWidth"] = WindowWidth;
	config["Debug"] = EnableDebug;
	config["SaveMissingEventName"] = SaveMissingEvent;
	config["ScreenshotSavePath"] = std::filesystem::path(ScreenshotSavePath.begin(), ScreenshotSavePath.end()).u8string();
	config["FontName"] = std::filesystem::path(FontName.begin(), FontName.end()).u8string();
	config["FontSize"] = FontSize;
	config["IsHideNoneChoise"] = IsHideNoneChoise;
	config["IsShowStatusBar"] = IsShowStatusBar;
	config["OptionMaxLine"] = OptionMaxLine;
	config["ImageType"] = ImageType;
	config["EnableCheckUpdate"] = EnableCheckUpdate;
	config["OcrPoolSize"] = OcrPoolSize;
	config["Theme"] = Theme;
	config["CaptureMode"] = (int)CaptureMode;
	config["Language"] = std::filesystem::path(Language.begin(), Language.end()).u8string();
	config["UpdateUrl"] = std::filesystem::path(UpdateUrl.begin(), UpdateUrl.end()).u8string();

	std::ofstream output(utility::GetExeDirectory() + L"\\config.json");
	output << std::setw(4) << config << std::endl;
}

std::wstring Config::GetImageExtension()
{
	switch (ImageType) {
		case 0:
			return L".png";
		case 1:
			return L".jpg";
		default:
			return L".png";
	}
}

std::wstring Config::GetImageMimeType()
{
	switch (ImageType) {
		case 0:
			return L"image/png";
		case 1:
			return L"image/jpeg";
		default:
			return L"image/png";
	}
}

Config* Config::GetInstance()
{
	static Config config;

	return &config;
}

