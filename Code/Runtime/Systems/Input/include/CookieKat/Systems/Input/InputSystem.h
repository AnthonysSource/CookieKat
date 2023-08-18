#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"

#include "CookieKat/Systems/EngineSystem/IEngineSystem.h"

#include "CookieKat/Systems/Input/InputDevice.h"
#include "CookieKat/Systems/Input/Devices/KeyboardDevice.h"
#include "CookieKat/Systems/Input/Devices/Mouse.h"

namespace CKE {
	// Engine Core Input Management System
	//-----------------------------------------------------------------------------
	class InputSystem : public IEngineSystem
	{
	public:
		void Initialize();
		void EndOfFrameUpdate();

		// Called by the platform to forward the input messages to the input system
		void ProcessPlatformInputMessage(PlatformInputMessage const& msg);

		inline KeyboardDevice const& GetKeyboard() const { return m_Keyboard; }
		inline Mouse const&          GetMouse() const { return m_Mouse; }

	private:
		Mouse          m_Mouse{};
		KeyboardDevice m_Keyboard{};
	};

	// Read-only input interface for the game systems
	//-----------------------------------------------------------------------------
	class InputContext
	{
	public:
		InputContext() = default;
		InputContext(Mouse const* pMouse, KeyboardDevice const* pKeyboard) : m_pMouse(pMouse), m_pKeyboard(pKeyboard) {}

		inline KeyboardDevice const* GetKeyboard() const;
		inline Mouse const*          GetMouse() const;

	private:
		Mouse const*          m_pMouse = nullptr;
		KeyboardDevice const* m_pKeyboard = nullptr;
	};
}


namespace CKE {
	KeyboardDevice const* InputContext::GetKeyboard() const {
		CKE_ASSERT(m_pKeyboard != nullptr);
		return m_pKeyboard;
	}

	Mouse const* InputContext::GetMouse() const {
		CKE_ASSERT(m_pMouse != nullptr);
		return m_pMouse;
	}
}
