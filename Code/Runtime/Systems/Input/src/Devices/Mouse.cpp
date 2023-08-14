#include "Devices/Mouse.h"

namespace CKE
{
	void Mouse::ProcessPlatformMessage(PlatformInputMessage const& msg)
	{
		switch (msg.m_Data1)
		{
		case 0:
		{
			// Mouse Move Message
			f32 xPos = static_cast<f32>(msg.m_Data2);
			f32 yPos = static_cast<f32>(msg.m_Data3);

			m_DeltaPos.x = xPos - m_Position.x;
			m_DeltaPos.y = yPos - m_Position.y;

			m_Position.x = xPos;
			m_Position.y = yPos;
		}
		case 1:
		{
			// Mouse Button Message
			i32 button = static_cast<f32>(msg.m_Data2);
			i32 action = static_cast<f32>(msg.m_Data3);
		}
		case 2: {
		}


		}

	}

	void Mouse::EndOfFrameUpdate()
	{
		m_DeltaPos = Vec2{ 0.0f };
	}
}
