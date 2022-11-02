#pragma once

#include <guiddef.h>

namespace utility {
	std::wstring GetExeDirectory();
	std::string to_string(const std::wstring &str);
	std::wstring ConvertUtf8ToUtf16(const char* str);
	std::wstring replace(const std::wstring& str, const std::wstring& src, const std::wstring& replaceStr);
	int GetEncoderClsid(const wchar_t* format, CLSID* pClsid);
}