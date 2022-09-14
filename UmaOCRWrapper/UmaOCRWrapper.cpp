#include "pch.h"

#include <msclr/marshal_cppstd.h>
#include "UmaOCRWrapper.h"

using namespace msclr::interop;
using namespace System;

void DECLSPEC InitOCR()
{
	UmaOCRDll::OCR::Init();
}

bool DECLSPEC RecognizeText(int width, int height, unsigned char* pixels, int size, int stride, std::wstring& output)
{
	String^ str = UmaOCRDll::OCR::RecognizeText(width, height, IntPtr(pixels), size, stride);

	output = marshal_as<std::wstring>(str);
	
	return !output.empty();
}
