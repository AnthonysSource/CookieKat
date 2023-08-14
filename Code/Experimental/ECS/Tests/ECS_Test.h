#pragma once

#include "CookieKat/Systems/ECS/IDs.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "BaseComponents.h"
#include "Base_Test.h"

#include <iostream>
#include <chrono>
#include <functional>

namespace CKE::ObjectModelTests {
	class ECS_Test
	{
	public:
		//-----------------------------------------------------------------------------

		inline void IteratePos() {
			for (auto pos : m_Admin.GetSingleCompIter(m_PositionID)) {
				Pos_Test((PositionComponent*)pos);
			}
		}

		inline void IteratePosVel() {
			for (ComponentTuple* compTuple : m_Admin.GetMultiCompIter({m_PositionID, m_VelocityID})) {
				auto pPos = compTuple->GetComponent<PositionComponent>(0);
				auto pVel = compTuple->GetComponent<VelocityComponent>(1);

				PosVel_Test(pPos, pVel);
			}
		}

		inline void IteratePosVelAccel() {
			for (ComponentTuple* compTuple : m_Admin.GetMultiCompIter({m_PositionID, m_VelocityID, m_AccelerationID})) {
				auto pPos = compTuple->GetComponent<PositionComponent>(0);
				auto pVel = compTuple->GetComponent<VelocityComponent>(1);
				auto pAccel = compTuple->GetComponent<AccelerationComponent>(2);

				PosVelAccel_Test(pPos, pVel, pAccel);
			}
		}

		//-----------------------------------------------------------------------------

		inline void Setup(TestParameters const testParameters) {
			m_PositionID = m_Admin.RegisterComponent<PositionComponent>();
			m_AccelerationID = m_Admin.RegisterComponent<AccelerationComponent>();
			m_VelocityID = m_Admin.RegisterComponent<VelocityComponent>();

			for (int i = 1; i <= testParameters.m_EntitiesPerBucket; ++i) {
				EntityID entity = m_Admin.CreateEntity();

				m_Admin.AddComponent<PositionComponent>(entity, 1.0f, 1.0f, 1.0f);
			}

			for (int i = 0; i < testParameters.m_EntitiesPerBucket; ++i) {
				EntityID entity = m_Admin.CreateEntity();

				PositionComponent pos{1.0f, 1.0f, 1.0f};
				VelocityComponent vel{2.0f, 2.0f, 2.0f};
				m_Admin.AddComponent(entity, m_PositionID, &pos);
				m_Admin.AddComponent(entity, m_VelocityID, &vel);
			}

			for (int i = 0; i < testParameters.m_EntitiesPerBucket; ++i) {
				EntityID entity = m_Admin.CreateEntity();

				PositionComponent     pos{1.0f, 1.0f, 1.0f};
				VelocityComponent     vel{2.0f, 2.0f, 2.0f};
				AccelerationComponent rot{2.0f, 2.0f, 2.0f};
				m_Admin.AddComponent(entity, m_PositionID, &pos);
				m_Admin.AddComponent(entity, m_VelocityID, &vel);
				m_Admin.AddComponent(entity, m_AccelerationID, &rot);
			}
		}

		inline void Run() {
			std::cout << "-------------------------------------------------------------\n";
			std::cout << "ECS: " << std::endl;
			RunTest("Pos", std::bind(&ECS_Test::IteratePos, this));
			RunTest("PosVel", std::bind(&ECS_Test::IteratePosVel, this));
			RunTest("PosVelAccel", std::bind(&ECS_Test::IteratePosVelAccel, this));
		}

	private:
		EntityDatabase m_Admin{1'100'000};
		ComponentTypeID    m_PositionID;
		ComponentTypeID    m_VelocityID;
		ComponentTypeID    m_AccelerationID;
	};
}
