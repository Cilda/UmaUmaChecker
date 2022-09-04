#include "Tesseract.h"

bool Tesseract::Init()
{
	return api->Init(".\\tessdata", "jpn");
}

std::wstring Tesseract::GetText()
{
	api->SetImage();
}
