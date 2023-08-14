#include "CookieKat/Core/Platform/PlatformTime.h"

#include "CookieKat/Core/Time/EngineTime.h"

#include "CookieKat/Core/Profilling/Profilling.h"
#include "CookieKat/Core/Math/Math.h"

#include <stdio.h>

namespace CKE
{
	void EngineTime::Initialize()
	{
		m_StartTick = PlatformTime::GetHighResolutionTicks();
	}

	void EngineTime::Update()
	{
		CKE_PROFILE_EVENT();

		// TODO: Handle engine time and game world time
		i64 newTick = PlatformTime::GetHighResolutionTicks();
		m_DeltaSeconds = static_cast<f32>(newTick - m_CurrentTick) / static_cast<f32>(
			PlatformTime::GetTicksFrequency());
		m_CurrentTick = newTick;
		m_SecondsUpTime += m_DeltaSeconds;

		// Increment frame number
		m_FrameNumber++;

		// TEMP: Cap delta time at 33ms
		m_DeltaSeconds = glm::clamp(m_DeltaSeconds, 0.0f, 0.033f);
	}
}
