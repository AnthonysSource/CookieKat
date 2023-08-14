#pragma once

namespace CKE {
	// Some global rendering settings
	struct RenderSettings
	{
		static constexpr u32  MAX_FRAMES_IN_FLIGHT = 2;
		static constexpr u32  MAX_OBJECTS = 27'100;
		static constexpr i32  GRAPHICS_CMDLIST_COUNT_PERFRAME = 100;
		static constexpr i32  TRANSFER_CMDLIST_COUNT_PERFRAME = 100;
		static constexpr i32  COMPUTE_CMDLIST_COUNT_PERFRAME = 50;
		static constexpr bool ENABLE_DEBUG = true;
	};
}