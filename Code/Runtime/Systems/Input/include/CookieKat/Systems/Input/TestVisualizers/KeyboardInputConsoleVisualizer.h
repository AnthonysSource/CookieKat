#pragma once

#include "CookieKat/Systems/Input/Devices/KeyboardDevice.h"

namespace CKE
{
	// Console visualizer to test that keyboard keypresses are being
	// registered correctly in the input system
	// 
	// Run the visualizer update after the platform input update
	// but before the engine update
	class KeyboardInputVisualizer
	{
	public:
		static void Update(KeyboardDevice const* keyboard, KeyCode key);
	};
}