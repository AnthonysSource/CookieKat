#pragma once

#include "CookieKat/Core/Profilling/Profilling.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/TaskSystem/TaskSystem.h"

//-----------------------------------------------------------------------------

namespace CKE {
	template <typename JobType, typename Component, typename... OtherComponents>
	class ECSJob : public ITaskSet
	{
	public:
		void Setup(EntityDatabase* pAdmin, f32 dt);
		//void ForEach(Component* comp, OtherComponents*... other) {}

	protected:
		f32 m_Dt{};

	private:
		void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override;

	private:
		EntityDatabase* m_pEntityDb = nullptr;
	};
}

//-----------------------------------------------------------------------------

namespace CKE {
	template <typename JobType, typename T, typename... Other>
	void ECSJob<JobType, T, Other...>::Setup(EntityDatabase* pAdmin, f32 dt) {
		m_pEntityDb = pAdmin;
		m_Dt = dt;
	}

	template <typename JobType, typename T, typename... Other>
	void ECSJob<JobType, T, Other...>::ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) {
		CKE_PROFILE_EVENT(typeid(JobType).name());

		JobType* pJob = static_cast<JobType*>(this);
		for (auto& compTuple : m_pEntityDb->GetMultiCompTupleIter<T, Other...>()) {
			std::apply([pJob](auto&&... args) { pJob->ForEach(args...); }, compTuple);
		}
	}
}
