#pragma once

#include "CookieKat/Core/Platform/PlatformTime.h"

namespace CKE
{
	class InputSystem;

	// Generic data structure that contains the platform
	// specific message with input data
	//
	// This will be decoded inside the platform specific input device
	// implementation
	// This structure allows for a more generic input system
	struct PlatformInputMessage
	{
		i64 m_Data0;
		i64 m_Data1;
		i64 m_Data2;
		i64 m_Data3;
		i64 m_Data4;
	};

	class InputDevice
	{
		friend InputSystem;

	public:
		virtual ~InputDevice() = default;

	protected:
		virtual void ProcessPlatformMessage(PlatformInputMessage const& msg) {}
		virtual void EndOfFrameUpdate() {};
	};
}