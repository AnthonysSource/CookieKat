#include "CookieKat/Core/Platform/PlatformTime.h"
#include "CookieKat/Core/Platform/Platform_Win32.h"

namespace CKE
{
	i64 PlatformTime::GetHighResolutionTicks()
	{
		LARGE_INTEGER value;
		QueryPerformanceCounter(&value);
		return value.QuadPart;
	}

	i64 PlatformTime::GetTicksFrequency()
	{
		LARGE_INTEGER value;
		QueryPerformanceFrequency(&value);
		return value.QuadPart;
	}
}
