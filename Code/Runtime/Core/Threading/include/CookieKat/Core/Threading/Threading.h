#pragma once

#include "CookieKat/Core/Platform/PrimitiveTypes.h"

#include <thread>
#include <mutex>

namespace CKE::Threading {
	struct CPUInfo
	{
		u16 m_PhysicalCoresCount = 0;
		u16 m_LogicalCoresCount = 0;
	};

	//-----------------------------------------------------------------------------

	using ThreadID = u32;
	using Thread = std::thread;
	using Mutex = std::mutex;
	using RecursiveMutex = std::recursive_mutex;
	using ConditionVariable = std::condition_variable;

	using Lock = std::unique_lock<Mutex>;
	using RecursiveLock = std::unique_lock<RecursiveMutex>;

	// Utilities
	//-----------------------------------------------------------------------------

	// Sleeps the calling thread
	inline void Sleep(u64 milliseconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	// Threading System
	//-----------------------------------------------------------------------------

	// Sets the calling thread as the main thread
	void InitializeMainThread();
	void Shutdown();

	// Returns true if the calling thread is the main thread
	bool IsMainThread();

	// Returns the ID of the calling thread
	ThreadID GetCurrentThreadID();
}
