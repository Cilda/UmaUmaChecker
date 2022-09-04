#pragma once

#include <tesseract/baseapi.h>

class Tesseract
{
public:
	Tesseract();
	~Tesseract();

	bool Init();
	std::wstring GetText();

private:
	tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
};

