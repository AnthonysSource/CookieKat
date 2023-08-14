#include "CookieKat/Core/Random/Random.h"

#include <random>

namespace CKE
{
	u32 Random::U32()
	{
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<u32> dist(0, std::numeric_limits<u32>::max());
		return dist(engine);
	}

	i32 Random::I32()
	{
		std::random_device rd;
		std::mt19937 eng(rd());
		std::uniform_int_distribution<i32> dist(std::numeric_limits<i32>::min(), std::numeric_limits<i32>::max());
		return dist(eng);
	}

	f32 Random::F32()
	{
		return F32(-FLT_MAX, FLT_MAX);
	}

	f64 Random::F64()
	{
		return F64(-DBL_MAX, DBL_MAX);
	}

	f32 Random::F32(f32 min, f32 max)
	{
		return (static_cast<f32>(std::rand()) / RAND_MAX) * (max - min) + min;
	}

	f64 Random::F64(f64 min, f64 max) {
		return (static_cast<f64>(std::rand()) / RAND_MAX) * (max - min) + min;
	}
}
