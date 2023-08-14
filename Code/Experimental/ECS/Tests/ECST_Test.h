#pragma once

#include "CookieKat/Core/Containers/Containers.h"

#include "CookieKat/Systems/ECS/IDs.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "BaseComponents.h"
#include "Base_Test.h"

#include <iostream>
#include <functional>

namespace CKE::ObjectModelTests {
	class ECST_Test
	{
	public:
		//-----------------------------------------------------------------------------

		inline void Iterate_Pos() {
			for (auto pos : m_Admin.GetSingleCompIter<PositionComponent>()) {
				Pos_Test(pos);
			}
		}

		inline void Iterate_PosVel() {
			for (ComponentTuple* compTuple : m_Admin.GetMultiCompIter<PositionComponent, VelocityComponent>()) {
				auto pPos = compTuple->GetComponent<PositionComponent>(0);
				auto pVel = compTuple->GetComponent<VelocityComponent>(1);

				PosVel_Test(pPos, pVel);
			}
		}

		inline void Iterate_PosVelAccel() {
			for (ComponentTuple* compTuple : m_Admin.GetMultiCompIter<PositionComponent, VelocityComponent, AccelerationComponent>()) {
				auto pos = compTuple->GetComponent<PositionComponent>(0);
				auto vel = compTuple->GetComponent<VelocityComponent>(1);
				auto rot = compTuple->GetComponent<AccelerationComponent>(2);

				PosVelAccel_Test(pos, vel, rot);
			}
		}

		inline void Iterate_PosVelAccel_Tuple() {
			for (auto&& [pos, vel, rot] : m_Admin.GetMultiCompTupleIter<PositionComponent, VelocityComponent, AccelerationComponent>()) {
				PosVelAccel_Test(pos, vel, rot);
			}
		}

		inline void Iterate_PosVelAccel_ForEach() {
			m_Admin.ForEach([](PositionComponent* pos, VelocityComponent* vel, AccelerationComponent* rot) {
				PosVelAccel_Test(pos, vel, rot);
			});
		}

		//-----------------------------------------------------------------------------

		inline void Setup(TestParameters const testParams) {
			m_Admin.RegisterComponent<PositionComponent>();
			m_Admin.RegisterComponent<AccelerationComponent>();
			m_Admin.RegisterComponent<VelocityComponent>();

			for (int i = 1; i <= testParams.m_EntitiesPerBucket; ++i) {
				EntityID entity = m_Admin.CreateEntity();
				m_Admin.AddComponent<PositionComponent>(entity, 1.0f, 1.0f, 1.0f);
			}

			for (int i = 0; i < testParams.m_EntitiesPerBucket; ++i) {
				EntityID entity = m_Admin.CreateEntity();
				m_Admin.AddComponent<PositionComponent>(entity, 1.0f, 1.0f, 1.0f);
				m_Admin.AddComponent<VelocityComponent>(entity, 1.0f, 1.0f, 1.0f);
			}

			for (int i = 0; i < testParams.m_EntitiesPerBucket; ++i) {
				EntityID entity = m_Admin.CreateEntity();
				m_Admin.AddComponent<PositionComponent>(entity, 1.0f, 1.0f, 1.0f);
				m_Admin.AddComponent<VelocityComponent>(entity, 1.0f, 1.0f, 1.0f);
				m_Admin.AddComponent<AccelerationComponent>(entity, 1.0f, 1.0f, 1.0f);
			}
		}

		inline void Run() {
			std::cout << "-------------------------------------------------------------\n";
			std::cout << "ECS Templated API:\n";
			RunTest("Pos", std::bind(&ECST_Test::Iterate_Pos, this));
			RunTest("PosVel", std::bind(&ECST_Test::Iterate_PosVel, this));
			RunTest("PosVelAccel", std::bind(&ECST_Test::Iterate_PosVelAccel, this));

			RunTest("PosVelAccel_Tuple", std::bind(&ECST_Test::Iterate_PosVelAccel_Tuple, this));
			RunTest("PosVelAccel_ForEach", std::bind(&ECST_Test::Iterate_PosVelAccel_ForEach, this));
		}

	private:
		EntityDatabase m_Admin{1'100'000};
	};
}
