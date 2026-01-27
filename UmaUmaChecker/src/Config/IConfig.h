#pragma once

#include <filesystem>

class IConfig
{
public:
	virtual ~IConfig() = default;

	virtual void Load(const std::filesystem::path& path) = 0;
};

