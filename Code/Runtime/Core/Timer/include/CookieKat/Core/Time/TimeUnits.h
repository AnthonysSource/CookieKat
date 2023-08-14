#pragma once

#include "CookieKat/Core/Platform/PlatformTime.h"

namespace CKE
{
	// Forward Declarations
	class Second;
	class Miliseconds;
	class Microseconds;
	class Nanoseconds;

	class Seconds
	{
	private:
	};

	class Milliseconds
	{
	private:
		u64 m_Value
	};

	class Microseconds
	{
	public:
		Microseconds(Nanoseconds nanoseconds) : m_Value(nanoseconds * 1000ull)
		{
		}

	private:
		u64 m_Value;
	};

	class Nanoseconds
	{
	public:
		inline u64 ToU64() { return m_Value; }

	private:
		u64 m_Value;
	};
}
