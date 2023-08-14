#pragma once

#include "CookieKat/Systems/EngineSystem/IEngineSystem.h"

#include "CookieKat/Core/Platform/PlatformTime.h"

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"

#include "KeyCodes.h"
#include "InputDevice.h"

#include "Devices/Keyboard.h"
#include "Devices/Mouse.h"

namespace CKE
{
	// Engine Core Input Management System
	//-----------------------------------------------------------------------------
	class InputSystem : public IEngineSystem
	{
	public:
		void Initialize();
		void EndOfFrameUpdate();

		// Called by the platform to forward the input messages to the input system
		void ProcessPlatformInputMessage(PlatformInputMessage const& msg);

		inline Keyboard const& GetKeyboard() const { return m_Keyboard; }
		inline Mouse const& GetMouse() const { return m_Mouse; }

	private:
		Mouse m_Mouse{};
		Keyboard m_Keyboard{};
	};

	// Read-only input interface for the game systems
	//-----------------------------------------------------------------------------
	class InputContext
	{
	public:
		InputContext() = default;
		InputContext(Mouse const* pMouse, Keyboard const* pKeyboard) : m_pMouse(pMouse), m_pKeyboard(pKeyboard) {}

		inline Keyboard const* GetKeyboard() const { CKE_ASSERT(m_pKeyboard != nullptr); return m_pKeyboard; }
		inline Mouse const* GetMouse() const { CKE_ASSERT(m_pMouse != nullptr); return m_pMouse; }

	private:
		Mouse const* m_pMouse = nullptr;
		Keyboard const* m_pKeyboard = nullptr;
	};
}
