#pragma once
#include <chrono>
#include <iostream>

#include "CookieKat/Core/Platform/PrimitiveTypes.h"

namespace CKE {
	class ScopedTimer
	{
	public:
		ScopedTimer() : m_StartPoint{std::chrono::system_clock::now()} {}

		void EndAndLog() {
			auto endPoint = std::chrono::system_clock::now();
			std::cout << std::chrono::duration<f64>(endPoint - m_StartPoint).count() << " Seconds \n";
		}

		std::chrono::time_point<std::chrono::system_clock> m_StartPoint;
	};
}
