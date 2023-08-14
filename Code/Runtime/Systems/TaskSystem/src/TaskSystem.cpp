#include "TaskSystem.h"

#include "CookieKat/Core/Profilling/Profilling.h"
#include "CookieKat/Core/Containers/String.h"

#include "TaskScheduler.h"
#include "CookieKat/Core/Platform/PlatformTime.h"

#include "format"

namespace CKE
{
	//-----------------------------------------------------------------------------

	static void OnThreadStart(u32 threadNum)
	{
		String name = std::format("Worker {}", threadNum);
		OPTICK_START_THREAD(name.c_str());
	}

	static void OnThreadStop(u32 threadNum)
	{
		OPTICK_STOP_THREAD();
	}

	//-----------------------------------------------------------------------------

	void TaskSystem::Initialize()
	{
		enki::TaskSchedulerConfig config{};
		config.profilerCallbacks.threadStart = OnThreadStart;
		config.profilerCallbacks.threadStop = OnThreadStop;

		m_TaskScheduler.Initialize(config);
	}

	void TaskSystem::Shutdown()
	{
		m_TaskScheduler.WaitforAllAndShutdown();
	}
}