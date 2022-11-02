#include <string>

#include "utility.h"

#include <Windows.h>
#include <gdiplus.h>
#include <vector>
#include <algorithm>

namespace utility
{
	std::wstring GetExeDirectory()
	{
		wchar_t FileName[1024];

		GetModuleFileNameW(NULL, FileName, 1024);

		std::wstring path(FileName);
		size_t pos = path.find_last_of('\\');

		if (pos != std::wstring::npos) {
			path.erase(pos);
		}

		return path;
	}

	std::string to_string(const std::wstring& str)
	{
		std::vector<char> ret;

		int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

		ret.resize(len);
		WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, ret.data(), len, NULL, NULL);
		ret.resize(ret.size() - 1);

		return std::string(ret.data(), ret.size());
	}
	std::wstring ConvertUtf8ToUtf16(const char* str)
	{
		std::vector<wchar_t> ret;

		int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str, -1, ret.data(), len);
		ret.resize(ret.size() - 1);

		return std::wstring(ret.data(), ret.size());
	}
	std::wstring replace(const std::wstring& str, const std::wstring& src, const std::wstring& replaceStr)
	{
		std::wstring ret = str;
		int pos = 0;

		while ((pos = ret.find(src, pos)) != std::wstring::npos) {
			ret.replace(pos, src.length(), replaceStr);
			pos += replaceStr.length();
		}

		return ret;
	}

	int GetEncoderClsid(const wchar_t* format, CLSID* pClsid)
	{
		UINT  num = 0;
		UINT  size = 0;

		Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;

		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1;

		GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;
			}
		}

		free(pImageCodecInfo);
		return -1;
	}
}
