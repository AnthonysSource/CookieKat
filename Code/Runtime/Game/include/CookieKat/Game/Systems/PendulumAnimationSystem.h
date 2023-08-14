#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/ECS/Jobs/ECSJob.h"

#include "CookieKat/Engine/Entities/Components/LocalToWorldComponent.h"
#include "CookieKat/Systems/ECS/Systems/ECSBaseSystem.h"

namespace CKE {
	struct PendulumComponent
	{
		float m_Theta = 0;
		float m_ThetaMax = 3.1415f / 4.0f;
		float m_Gravity = 9.81f;
		float m_StringLength = 3;
		float m_CurrentTime = 0;
	};

	class PendulumAnimationSystem : public ECSBaseSystem
	{
	public:
		inline void Update(SystemUpdateContext ctx) override {
			CKE_PROFILE_EVENT();

			auto db = ctx.GetEntityDatabase();

			for (auto& [l2w, p] : db->GetMultiCompTupleIter<LocalToWorldComponent, PendulumComponent>()) {
				float x = p->m_StringLength * sin(p->m_Theta);
				float y = -p->m_StringLength * cos(p->m_Theta) + 2.5f;
				p->m_Theta = p->m_ThetaMax * sin(sqrt(p->m_Gravity / p->m_StringLength) * p->m_CurrentTime);
				p->m_CurrentTime += ctx.GetDeltaTime();

				l2w->m_LocalToWorld[3].x = x;
				l2w->m_LocalToWorld[3].y = y;
			}
		}
	};
}
