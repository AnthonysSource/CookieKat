#pragma once

#include "CookieKat/Core/Platform/PlatformTime.h"

namespace CKE
{
	class EngineTime
	{
		friend class Engine;

	public:
		inline u64 GetTickUpTime() const;

		// Returns the time in seconds since the engine initialized
		inline f32 GetSecondsUpTime() const { return m_SecondsUpTime; }
		inline f32 GetSecondsDeltaTime() const { return m_DeltaSeconds; }

		// Returns the frame number since the engine started
		inline u64 GetFrameNumber() const;

	private:
		void Initialize();
		void Update();

		i64 m_StartTick;
		i64 m_CurrentTick;

		f32 m_SecondsUpTime;
		f32 m_DeltaSeconds;

		u64 m_FrameNumber;
	};

	struct GameClock
	{
		i64 m_StartTick;
		i64 m_CurrentTick;

		f32 m_SecondsUptime;
		f32 m_DeltaSeconds;
	};

	inline u64 EngineTime::GetTickUpTime() const { return m_CurrentTick - m_StartTick; }
	inline u64 EngineTime::GetFrameNumber() const { return m_FrameNumber; }
}
