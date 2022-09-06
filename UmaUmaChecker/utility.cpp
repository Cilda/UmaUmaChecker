#include <string>

#include "utility.h"

#include <Windows.h>

namespace utility {
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
}


