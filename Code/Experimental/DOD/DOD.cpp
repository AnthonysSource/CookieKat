// #define _ITERATOR_DEBUG_LEVEL 0

#include "CookieKat/Core/Platform/PlatformTime.h"

#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Random/Random.h"

#include <chrono>
#include <iostream>

using namespace CKE;

struct Transform
{
	f32 x;
	f32 y;
	f32 z;

	f32 vx;
	f32 vy;
	f32 vz;
};

constexpr u32 NUM_DATA = 1'000'000;

Vector<Transform> s_Packed;
Vector<Transform*> s_Sparse;

void Populate()
{
	s_Packed.resize(NUM_DATA);

	for (auto& d : s_Packed)
	{
		d.x = Random::F32(-100, 100);
		d.y = Random::F32(-100, 100);
		d.z = Random::F32(-100, 100);
		d.vx = Random::F32(-100, 100);
		d.vy = Random::F32(-100, 100);
		d.vz = Random::F32(-100, 100);
	}

	//s_Sparse.reserve(NUM_DATA);

	//for (int i = 0; i < NUM_DATA; ++i)
	//{
	//	auto vec = new Transform{};
	//	vec->x = Random::F32(-100, 100);
	//	vec->y = Random::F32(-100, 100);
	//	vec->z = Random::F32(-100, 100);
	//	s_Sparse.emplace_back(vec);

	//	//for (int i = 0; i < 250; ++i)
	//	//{
	//	//	new Transform{};
	//	//}
	//}
}

void PackedUpdate()
{
	for (int i = 0; i < NUM_DATA; ++i)
	{
		auto& pos = s_Packed[i];
		pos.x += pos.vx * 0.01f + sqrt(pos.x);
		pos.y += pos.vy * 0.01f + sqrt(pos.y);
		pos.z += pos.vz * 0.01f + sqrt(pos.z);
	}
}

void Run()
{
	while (true)
	{
		//{
		//	auto startTime = std::chrono::system_clock::now();

		//	for (int i = 0; i < NUM_DATA; ++i)
		//	{
		//		s_Packed[i].x += s_Packed[i].z * 2 + s_Packed[i].y;
		//	}

		//	auto endTime = std::chrono::system_clock::now();
		//	auto elapsed =
		//		std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		//	std::cout << elapsed << " - Packed Non-Cached Var" << std::endl;
		//}

		//{
		//	auto startTime = std::chrono::system_clock::now();

		//	for (int i = 0; i < NUM_DATA; ++i)
		//	{
		//		auto vec = s_Sparse[i];
		//		vec->x += 1.0f;
		//		vec->y += 1.0f;
		//		vec->z += 1.0f;
		//	}

		//	auto endTime = std::chrono::system_clock::now();
		//	auto elapsed =
		//		std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		//	std::cout << elapsed << " - Sparse Cached Var" << std::endl;
		//}

		{
			for (int i = 0; i < 100; ++i)
			{
				auto startTime = std::chrono::system_clock::now();

				PackedUpdate();

				auto endTime = std::chrono::system_clock::now();
				auto elapsed =
					std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

				std::cout << elapsed << " - Packed Cached Var" << std::endl;
			}
		}

		//{
		//	auto startTime = std::chrono::system_clock::now();

		//	for (int i = 0; i < NUM_DATA; ++i)
		//	{
		//		s_Sparse[i]->Update();
		//	}

		//	auto endTime = std::chrono::system_clock::now();
		//	auto elapsed =
		//		std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		//	std::cout << elapsed << " - Sparse Update" << std::endl;
		//}

		//{
		//	auto startTime = std::chrono::system_clock::now();

		//	for (int i = 0; i < NUM_DATA; ++i)
		//	{
		//		s_Packed[i].Update();
		//	}

		//	auto endTime = std::chrono::system_clock::now();
		//	auto elapsed =
		//		std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		//	std::cout << elapsed << " - Packed Update" << std::endl;
		//}

		std::cout << "--------------------------------------------------------" << std::endl;
		system("pause");
	}
}

int main()
{
	Map<u64, u64> map;
	for (int i = 0; i < 1'000'000; ++i)
	{
		map.insert({ i,i });
	}
	Vector<u64> out{ 1'000'000 };

	auto c1 = std::chrono::high_resolution_clock::now();


	for (int i = 0; i < 1'000'000; ++i)
	{
		int a = Random::U32();
		out[i] = map[a];
	}

	auto c2 = std::chrono::high_resolution_clock::now();
	auto f = std::chrono::duration_cast<std::chrono::milliseconds>(c2 - c1);
	std::cout << f << std::endl;

	std::cout << out[25342] << std::endl;

	// Populate();
	// Run();

	return 0;
}
