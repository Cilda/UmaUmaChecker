#pragma once

#include <string>

#ifdef DLL_EXPORT
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif

void DECLSPEC InitOCR();
bool DECLSPEC RecognizeText(int width, int height, unsigned char* pixels, int size, int stride, std::wstring& output);
