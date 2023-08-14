#pragma once

#include "CookieKat/Core/Platform/PlatformTime.h"
#include <random>

namespace CKE {
	// General purpose random number generation functions
	class Random
	{
	public:
		static u32 U32();
		static i32 I32();
		static f32 F32();
		static f64 F64();
		static f32 F32(f32 min, f32 max);
		static f64 F64(f64 min, f64 max);
	};
}
