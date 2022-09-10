#pragma once

namespace utility {
	std::wstring GetExeDirectory();
	std::string to_string(const std::wstring &str);
	std::wstring ConvertUtf8ToUtf16(const char* str);
}