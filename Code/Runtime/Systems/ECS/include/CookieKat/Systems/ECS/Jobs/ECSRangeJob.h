#include "Core/Profilling/Profilling.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/TaskSystem/TaskSystem.h"

namespace CKE
{
	//template <typename Component, typename... OtherComponents>
	//class ECSRangeJobDef
	//{
	//	void ForEach(Component* comp, OtherComponents*... other) {}
	//};

	//template <typename JobType, typename Component, typename... OtherComponents>
	//class ECSRangeJob : public ITaskSet
	//{
	//public:
	//	void Setup(u64 startRange, u64 endRange, EntityDatabase* admin, f32 dt);
	//	void ForEach(Component* comp, OtherComponents*... other) {}

	//protected:
	//	f32 m_Dt{};

	//private:
	//	void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override;

	//private:
	//	EntityDatabase* m_pEntityDb = nullptr;
	//	u64 m_StartRange;
	//	u64 m_EndRange;
	//};

	//template <typename JobType, typename Component, typename... OtherComponents>
	//class ECSRangeJobScheduler
	//{
	//public:
	//	inline void Setup(u64 splitCount)
	//	{

	//	}

	//	inline void Run(TaskSystem* pTaskSystem)
	//	{
	//		for (ECSRangeJob& job : m_Jobs)
	//		{
	//			pTaskSystem->ScheduleTask(&job);
	//		}

	//		for (ECSRangeJob& job : m_Jobs)
	//		{
	//			pTaskSystem->WaitForTask(&job);
	//		}
	//	}

	//private:
	//	Vector<ECSRangeJob> m_Jobs;
	//};

	//-----------------------------------------------------------------------------

	template <typename JobType, typename Component, typename ... OtherComponents>
	void ECSRangeJob<JobType, Component, OtherComponents...>::Setup(u64 startRange, u64 endRange, EntityDatabase* admin, f32 dt)
	{
		m_pEntityDb = admin;
		m_Dt = dt;
		m_StartRange = startRange;
		m_EndRange = endRange;
	}

	template <typename JobType, typename Component, typename ... OtherComponents>
	void ECSRangeJob<JobType, Component, OtherComponents...>::ExecuteRange(enki::TaskSetPartition range_,
		uint32_t threadnum_)
	{
		CKE_PROFILE_EVENT(typeid(JobType).name());
		TMultiComponentIter iter = m_pEntityDb->GetMultiCompTupleIter<Component, OtherComponents...>().begin();
		iter + m_StartRange;
		u64 i = 0;

		JobType* pJob = static_cast<JobType*>(this);

		for (TMultiComponentIter it = iter; it != iter.end(); ++it) {
			auto& compTuple = *it;

			std::apply([pJob](auto &&... args) { pJob->ForEach(args...); }, compTuple);

			++i;
			if (i >= m_EndRange) { break; }
		}
	}
}