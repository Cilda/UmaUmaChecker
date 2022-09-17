#pragma once

#include <vector>

class TextCollector
{
public:
	struct Replacement {
		std::wstring Search;
		std::wstring Replace;

	public:
		Replacement(const std::wstring& Search, const std::wstring Replace) {
			this->Search = Search;
			this->Replace = Replace;
		}
	};

public:
	TextCollector();
	~TextCollector();

	void Load();

	void Collect(std::wstring& text);

private:
	std::vector<Replacement> collections;
};

