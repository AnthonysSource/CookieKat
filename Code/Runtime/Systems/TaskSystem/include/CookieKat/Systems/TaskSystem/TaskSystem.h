#pragma once

#include "CookieKat/Systems/EngineSystem/IEngineSystem.h"
#include "TaskScheduler.h"

namespace CKE {
	using ITaskSet = enki::ITaskSet;
	using IPinnedTask = enki::IPinnedTask;
}

namespace CKE {
	class TaskSystem : public IEngineSystem
	{
	public:
		// Lifetime
		//-----------------------------------------------------------------------------

		void Initialize();
		void Shutdown();

		// Tasks
		//-----------------------------------------------------------------------------

		inline void ScheduleTask(ITaskSet* taskSet) { m_TaskScheduler.AddTaskSetToPipe(taskSet); }
		inline void AddPinnedTask(IPinnedTask* pPinnedTask) { m_TaskScheduler.AddPinnedTask(pPinnedTask); }
		inline void RunPinnedTask() { m_TaskScheduler.RunPinnedTasks(); }
		inline void WaitForTask(ITaskSet* taskSet) { m_TaskScheduler.WaitforTask(taskSet); }

		inline enki::TaskScheduler* GetScheduler() { return  &m_TaskScheduler; }

	private:
		enki::TaskScheduler m_TaskScheduler;
	};
}
