#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Systems/Input/InputDevice.h"

namespace CKE
{
	class Mouse : public InputDevice
	{
	public:
		void ProcessPlatformMessage(PlatformInputMessage const& msg) override;
		void EndOfFrameUpdate() override;

	public:
		Vec2 m_Position;
		Vec2 m_DeltaPos;
		f32 m_ScrollYOffset;

		static constexpr i32 NUM_KEYS = 8;
		Array<bool, NUM_KEYS> m_Pressed;
		Array<bool, NUM_KEYS> m_Held;
		Array<bool, NUM_KEYS> m_Released;
	};
}