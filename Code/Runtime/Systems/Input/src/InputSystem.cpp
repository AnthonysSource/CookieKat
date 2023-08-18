#include "InputSystem.h"

#include "TestVisualizers/KeyboardInputConsoleVisualizer.h"
#include "CookieKat/Core/Profilling/Profilling.h"

namespace CKE {
	void InputSystem::Initialize() { }

	void InputSystem::EndOfFrameUpdate() {
		CKE_PROFILE_EVENT();
		// KeyboardInputVisualizer::Update(&m_Keyboard, KeyCode::A);

		m_Keyboard.EndOfFrameUpdate();
		m_Mouse.EndOfFrameUpdate();
	}

	void InputSystem::ProcessPlatformInputMessage(PlatformInputMessage const& msg) {
		u64 msgType = msg.m_Data0;

		switch (msgType) {
		case 0: {
			m_Keyboard.ProcessPlatformMessage(msg);
			break;
		}
		case 1: {
			m_Mouse.ProcessPlatformMessage(msg);
			break;
		}
		}
	}
}
