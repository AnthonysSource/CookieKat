#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/ECS/Jobs/ECSJob.h"

#include "CookieKat/Engine/Entities/Components/LocalToWorldComponent.h"
#include "CookieKat/Systems/ECS/Systems/ECSBaseSystem.h"

namespace CKE {
	class CubeMoverJob : public ECSJob<CubeMoverJob, LocalToWorldComponent, VelocityComponent>
	{
	public:
		inline void ForEach(LocalToWorldComponent* l2w, VelocityComponent* vel) {
			Vec3 pos = l2w->m_LocalToWorld[3];

			Vec3 movDir = -pos;
			if (glm::length2(movDir) > 0.001f) {
				movDir = glm::normalize(movDir);
			}

			vel->m_Velocity += m_Dt * Vec3(0.1f) * movDir;

			if (glm::length2(vel->m_Velocity) > 25.0f) {
				vel->m_Velocity = glm::normalize(vel->m_Velocity) * 5.0f;
			}

			l2w->m_LocalToWorld[3].x += vel->m_Velocity.x;
			l2w->m_LocalToWorld[3].y += vel->m_Velocity.y;
			l2w->m_LocalToWorld[3].z += vel->m_Velocity.z;
		}
	};

	class CubeMoverSystem : public ECSBaseSystem
	{
	public:
		inline void Update(SystemUpdateContext ctx) override {
			CKE_PROFILE_EVENT();

			TaskSystem* pTaskSys = ctx.GetTaskSystem();

			CubeMoverJob cubeMoverJob;
			cubeMoverJob.Setup(ctx.GetEntityDatabase(), ctx.GetDeltaTime());

			pTaskSys->ScheduleTask(&cubeMoverJob);
			pTaskSys->WaitForTask(&cubeMoverJob);
		}
	};
}

namespace CKE {
	/*class CubeMoverSystemThreaded : public ECSBaseSystem
	{
	public:
		inline void Update(EntityDatabase* pEntityDB, TaskSystem* pTaskSystem, f32 dt) override
		{
			struct MoveCubesJob : ITaskSet
			{
				void Setup(u64 startRange, u64 endRange, EntityDatabase* admin, f32 dt) {
					m_Admin = admin;
					m_Dt = dt;
					m_StartRange = startRange;
					m_EndRange = endRange;
				}

				EntityDatabase* m_Admin;
				f32 m_Dt;
				u64 m_StartRange;
				u64 m_EndRange;

				inline void ForEach(LocalToWorldComponent* l2w, VelocityComponent* velComp)
				{
					Vec3 pos = l2w->m_LocalToWorld[3];

					Vec3 movDir = -pos;
					if (glm::length(movDir) > 0.01f)
					{
						movDir = glm::normalize(movDir);
					}

					velComp->m_Velocity += m_Dt * Vec3(1.5f) * movDir;

					if (glm::length(velComp->m_Velocity) > 5.0f)
					{
						velComp->m_Velocity = glm::normalize(velComp->m_Velocity) * 5.0f;
					}

					l2w->m_LocalToWorld[3].x += velComp->m_Velocity.x;
					l2w->m_LocalToWorld[3].y += velComp->m_Velocity.y;
					l2w->m_LocalToWorld[3].z += velComp->m_Velocity.z;
				}

				void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override
				{
					CKE_PROFILE_EVENT("Cube System Task");
					MultiComponentIter iter = m_Admin->GetMultiCompIter<LocalToWorldComponent, VelocityComponent>().begin();
					iter + m_StartRange;
					u64 i = 0;
					for (MultiComponentIter it = iter; it != iter.end(); ++it) {
						auto pCompIter = *it;
						auto l2w = pCompIter->GetComponent<LocalToWorldComponent>(0);
						auto velComp = pCompIter->GetComponent<VelocityComponent>(1);

						ForEach(l2w, velComp);

						++i;
						if (i >= m_EndRange) { break; }
					}
				}
			};

			constexpr u32 size = 40;
			constexpr u32 taskSize = 25'000;
			MoveCubesJob jobs[size];
			for (u64 i = 0; i < size; ++i)
			{
				jobs[i].Setup(i * taskSize, taskSize, pEntityDB, dt);
				pTaskSystem->ScheduleTask(&jobs[i]);
			}

			for (u64 i = 0; i < size; ++i)
			{
				pTaskSystem->WaitForTask(&jobs[i]);
			}
		}
	};*/
}
