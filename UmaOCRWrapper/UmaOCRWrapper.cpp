#include "pch.h"

#include <msclr/marshal_cppstd.h>
#include "UmaOCRWrapper.h"

using namespace msclr::interop;
using namespace System;

void DECLSPEC InitOCR()
{
	UmaOCRDll::OCR::Init();
}

bool DECLSPEC RecognizeText(int width, int height, unsigned char* pixels, int size, int stride, wchar_t* pOutput, size_t outputSize)
{
	String^ str = UmaOCRDll::OCR::RecognizeText(width, height, IntPtr(pixels), size, stride);

	marshal_context ctx;
	const wchar_t *result = ctx.marshal_as<const wchar_t*>(str);

	wcscpy_s(pOutput, outputSize, result);
	
	return str->Length > 0;
}

bool DECLSPEC RecognizeTextFromGrayImage(int width, int height, unsigned char* pixels, int size, int stride, wchar_t* pOutput, size_t outputSize)
{
	String^ str = UmaOCRDll::OCR::RecognizeTextFromGrayImage(width, height, IntPtr(pixels), size, stride);

	marshal_context ctx;
	const wchar_t* result = ctx.marshal_as<const wchar_t*>(str);

	wcscpy_s(pOutput, outputSize, result);

	return str->Length > 0;
}