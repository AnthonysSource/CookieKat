#include "Devices/Keyboard.h"

#include "GLFW/glfw3.h"

namespace CKE
{
	bool Keyboard::GetKeyPressed(KeyCode keyCode) const
	{
		return m_Pressed[static_cast<i32>(keyCode)];
	}

	bool Keyboard::GetKeyHeld(KeyCode keyCode) const
	{
		return m_Held[static_cast<i32>(keyCode)];
	}

	bool Keyboard::GetKeyReleased(KeyCode keyCode) const
	{
		return m_Released[static_cast<i32>(keyCode)];
	}

	void Keyboard::ProcessPlatformMessage(PlatformInputMessage const& msg)
	{
		i32 const key = static_cast<i32>(msg.m_Data1);
		i32 const scancode = static_cast<i32>(msg.m_Data2);
		i32 const action = static_cast<i32>(msg.m_Data3);
		i32 const mod = static_cast<i32>(msg.m_Data4);

		if (action == GLFW_PRESS)
		{
			m_Pressed[key] = true;
			m_Held[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_Held[key] = false;
			m_Released[key] = true;
		}
	}

	void Keyboard::EndOfFrameUpdate()
	{
		for (auto& pressed : m_Pressed)
		{
			pressed = false;
		}

		for (auto& released : m_Released)
		{
			released = false;
		}
	}
}
