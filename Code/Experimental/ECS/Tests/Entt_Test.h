#pragma once

#include "BaseComponents.h"
#include "Base_Test.h"
#include "entt/entt.hpp"

#include <iostream>
#include <functional>

namespace CKE::ObjectModelTests {
	using namespace CKE::ECS;

	class Entt_Test
	{
	public:
		//-----------------------------------------------------------------------------

		inline void Iterate_Pos() {
			auto view = m_Registry.view<PositionComponent>();

			for (auto [entity, pos] : view.each()) {
				Pos_Test(&pos);
			}
		}

		inline void Iterate_PosVel() {
			auto view = m_Registry.view<PositionComponent, VelocityComponent>();

			for (auto [entity, pos, vel] : view.each()) {
				PosVel_Test(&pos, &vel);
			}
		}

		inline void Iterate_PosVelAccel() {
			auto view = m_Registry.view<PositionComponent, VelocityComponent, AccelerationComponent>();

			for (auto [entity, pos, vel, rot] : view.each()) {
				PosVelAccel_Test(&pos, &vel, &rot);
			}
		}

		inline void Iterate_PosVelAccel_Lambda() {
			auto view = m_Registry.view<PositionComponent, VelocityComponent, AccelerationComponent>();

			view.each([](auto& pos, auto& vel, auto& rot) {
				PosVelAccel_Test(&pos, &vel, &rot);
			});
		}

		//-----------------------------------------------------------------------------

		inline void Setup(TestParameters const testParams) {
			for (int i = 0; i < testParams.m_EntitiesPerBucket; ++i) {
				auto entity = m_Registry.create();
				m_Registry.emplace<PositionComponent>(entity, 0.01f + 0.1f * i, 0.01f + 0.1f * i, 0.01f + 0.1f * i);
			}

			for (int i = 0; i < testParams.m_EntitiesPerBucket; ++i) {
				auto entity = m_Registry.create();
				m_Registry.emplace<PositionComponent>(entity, 0.01f + 0.1f * i, 0.01f + 0.1f * i, 0.01f + 0.1f * i);
				m_Registry.emplace<VelocityComponent>(entity, 0.01f + 0.1f * i, 0.01f + 0.1f * i, 0.01f + 0.1f * i);
			}

			for (int i = 0; i < testParams.m_EntitiesPerBucket; ++i) {
				auto entity = m_Registry.create();
				m_Registry.emplace<PositionComponent>(entity, 0.01f + 0.1f * i, 0.01f + 0.1f * i, 0.01f + 0.1f * i);
				m_Registry.emplace<VelocityComponent>(entity, 0.01f + 0.1f * i, 0.01f + 0.1f * i, 0.01f + 0.1f * i);
				m_Registry.emplace<AccelerationComponent>(entity, 0.01f + 0.1f * i, 0.01f + 0.1f * i, 0.01f + 0.1f * i);
			}
		}

		inline void Run() {
			std::cout << "-------------------------------------------------------------\n";
			std::cout << "Entt:\n";
			RunTest("Pos", std::bind(&Entt_Test::Iterate_Pos, this));
			RunTest("PosVel", std::bind(&Entt_Test::Iterate_PosVel, this));
			RunTest("PosVelAccel", std::bind(&Entt_Test::Iterate_PosVelAccel, this));
			RunTest("PosVelAccel_Lambda", std::bind(&Entt_Test::Iterate_PosVelAccel_Lambda, this));
		}

	private:
		entt::registry m_Registry;
	};
}
