#pragma once

#include <Pdh.h>

class SystemUsage
{
private:
	SystemUsage();
	~SystemUsage();

public:
	static SystemUsage& Get();

	double GetCpuUsage();
	size_t MemoryUsage() const;

private:
	static UINT64 FileTimeToUTC(const FILETIME& ftime);

private:
	INT64 LastSystemTime;
	INT64 LastCpuTime;
	int ProcessorCount;
};

