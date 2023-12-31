#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Systems/Input/InputDevice.h"
#include <CookieKat/Systems/Input/KeyCodes.h>

namespace CKE {
	class KeyboardDevice : public InputDevice
	{
	public:
		bool GetKeyPressed(KeyCode keyCode) const;
		bool GetKeyHeld(KeyCode keyCode) const;
		bool GetKeyReleased(KeyCode keyCode) const;

	protected:
		void ProcessPlatformMessage(PlatformInputMessage const& msg) override;
		void EndOfFrameUpdate() override;

	private:
		friend class InputSystem;

		static constexpr i32  NUM_KEYS = 350;
		Array<bool, NUM_KEYS> m_Pressed;
		Array<bool, NUM_KEYS> m_Held;
		Array<bool, NUM_KEYS> m_Released;
	};
}
