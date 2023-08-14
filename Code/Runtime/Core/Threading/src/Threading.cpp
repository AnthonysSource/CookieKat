#include "CookieKat/Core/Threading/Threading.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace CKE::Threading {
	static ThreadID s_MainThreadID = 0;

	bool IsMainThread() {
		return GetCurrentThreadID() == s_MainThreadID;
	}

	void InitializeMainThread() {
		s_MainThreadID = GetCurrentThreadID();
	}

	void Shutdown() {
		s_MainThreadID = 0;
	}

	ThreadID GetCurrentThreadID() {
		HANDLE threadHandle = GetCurrentThread();
		ThreadID const nativeThreadID = GetThreadId(threadHandle);
		return nativeThreadID;
	}
}
