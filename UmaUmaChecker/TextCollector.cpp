#include <string>

#include "TextCollector.h"

#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>

#include "utility.h"

using json = nlohmann::json;

TextCollector::TextCollector()
{
}

TextCollector::~TextCollector()
{
}

void TextCollector::Load()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\ReplaceText.json");
	if (stream.good()) {
		std::stringstream text;

		text << stream.rdbuf();

		try {
			json replaces = json::parse(text.str());

			for (auto& arr : replaces["replaces"]) {
				collections.emplace_back(
					utility::ConvertUtf8ToUtf16(arr["search"].get<std::string>().c_str()),
					utility::ConvertUtf8ToUtf16(arr["replace"].get<std::string>().c_str())
				);
			}
		}
		catch (json::exception& ex) {
		}
	}
}

bool TextCollector::Collect(std::wstring& text)
{
	std::wstring org = text;

	for (auto& c : collections) {
		std::wregex reg(c.Search);

		text = std::regex_replace(text, reg, c.Replace);
	}

	return text != org;
}
