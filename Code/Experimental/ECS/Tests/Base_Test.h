#pragma once

#include <chrono>
#include <functional>
#include <iostream>

#include "./BaseComponents.h"

namespace CKE::ObjectModelTests {
	using namespace CKE::ECS;

	struct TestParameters
	{
		u32 m_EntitiesPerBucket = 250'000;
	};

	inline void RunTest(const char* testName, std::function<void()> testFunc) {
		constexpr u64 ITER_COUNT = 120;

		float averageEntt = 0;
		for (int i = 0; i < ITER_COUNT; ++i) {
			auto start = std::chrono::system_clock::now();

			testFunc();

			auto end = std::chrono::system_clock::now();
			auto elapsed =
					std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			averageEntt += elapsed.count();
		}
		std::cout << testName << " - " << averageEntt / ITER_COUNT << "ms" << std::endl;
	}
}

namespace CKE::ObjectModelTests {
	inline void Pos_Test(PositionComponent* pPos) {
		pPos->x += 1.0f;
		pPos->y += 1.0f;
		pPos->z += 1.0f;
	}

	inline void PosVel_Test(PositionComponent* pPos, VelocityComponent* pVel) {
		pVel->x += 1.0f;
		pVel->y += 1.0f;
		pVel->z += 1.0f;

		pPos->x += pVel->x * 0.1f;
		pPos->y += pVel->y * 0.1f;
		pPos->z += pVel->z * 0.1f;
	}

	inline void PosVelAccel_Test(PositionComponent* pPos, VelocityComponent* pVel, AccelerationComponent* pAccel) {
		pAccel->x += 1.0f - std::sqrt(pVel->x) * 0.1f;
		pAccel->y += 1.0f - std::sqrt(pVel->y) * 0.1f;
		pAccel->z += 1.0f - std::sqrt(pVel->z) * 0.1f;

		pVel->x += pAccel->x * 0.01f;
		pVel->y += pAccel->y * 0.01f;
		pVel->z += pAccel->z * 0.01f;

		pPos->x += pVel->x * 0.1f;
		pPos->y += pVel->y * 0.1f;
		pPos->z += pVel->z * 0.1f;
	}
}
