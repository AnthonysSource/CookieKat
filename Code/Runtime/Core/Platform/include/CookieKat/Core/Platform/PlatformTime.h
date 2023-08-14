#pragma once

#include "PrimitiveTypes.h"

namespace CKE
{
	// High-Resolution platform timer
	class PlatformTime
	{
	public:
		// Returns the current high resolution time of the platform
		static i64 GetHighResolutionTicks();

		// Return the update frequency of the timer
		static i64 GetTicksFrequency();
	};
}
