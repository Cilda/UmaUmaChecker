#include "SystemUsage.h"

#include <psapi.h>
#include <exception>
#include <thread>

SystemUsage::SystemUsage() : LastCpuTime(0), LastSystemTime(0)
{
	SYSTEM_INFO SysInfo;

	GetSystemInfo(&SysInfo);
	ProcessorCount = SysInfo.dwNumberOfProcessors;
}

SystemUsage::~SystemUsage()
{
}

SystemUsage& SystemUsage::Get()
{
	static SystemUsage usage;
	return usage;
}

double SystemUsage::GetCpuUsage()
{
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;

	if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time)) {
		return 0.0;
	}

	INT64 system_time = (FileTimeToUTC(kernel_time) + FileTimeToUTC(user_time)) / ProcessorCount;

	INT64 now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if (LastSystemTime == 0) {
		LastSystemTime = system_time;
		LastCpuTime = now;
		return 0.0;
	}

	INT64 system_time_delta = system_time - LastSystemTime;
	INT64 time_delta = (now - LastCpuTime) * 10;

	if (time_delta == 0) return 0.0;

	double cpu = ((system_time_delta * 100.0) / time_delta);
	LastSystemTime = system_time;
	LastCpuTime = now;

	return cpu;
}

size_t SystemUsage::MemoryUsage() const
{
	MEMORY_BASIC_INFORMATION mbi;
	void* pBaseAddrses = NULL;
	size_t private_bytes = 0;

	while (VirtualQueryEx(GetCurrentProcess(), pBaseAddrses, &mbi, sizeof(mbi))) {
		if (mbi.State == MEM_COMMIT) {
			if (mbi.Type == MEM_PRIVATE) {
				private_bytes += mbi.RegionSize;
			}
		}

		void* pNewBase = ((byte*)mbi.BaseAddress) + mbi.RegionSize;
		if (pNewBase <= pBaseAddrses) {
			break;
		}

		pBaseAddrses = pNewBase;
	}

	return private_bytes;
}

UINT64 SystemUsage::FileTimeToUTC(const FILETIME& ftime)
{
	LARGE_INTEGER li;

	li.LowPart = ftime.dwLowDateTime;
	li.HighPart = ftime.dwHighDateTime;
	return li.QuadPart;
}
