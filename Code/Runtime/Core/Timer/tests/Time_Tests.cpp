#include "CookieKat/Core/Platform/PlatformTime.h"

#include <gtest/gtest.h>

//-----------------------------------------------------------------------------

TEST(Core_Time, HighResolution_Timer) {
	using namespace CKE;

	auto RunIter = [] {
		i64  t1 = PlatformTime::GetHighResolutionTicks();
		auto timer1 = std::chrono::high_resolution_clock::now();
		int  k = 0;
		for (int i = 0; i < 1'000'000; ++i) {
			k += 1;
		}
		i64  t2 = PlatformTime::GetHighResolutionTicks();
		auto timer2 = std::chrono::high_resolution_clock::now();

		auto stdns1 = std::chrono::duration_cast<std::chrono::nanoseconds>(timer1.time_since_epoch());
		auto stdns2 = std::chrono::duration_cast<std::chrono::nanoseconds>(timer2.time_since_epoch());
		auto dt1 = stdns2.count() - stdns1.count();
		auto dt2 = (t2 - t1) * 1'000'000'000ll / PlatformTime::GetTicksFrequency();
		auto diff = abs(dt1 - dt2);

		return diff;
	};

	constexpr i64 maxLatencyDiff = 2000;

	// We run the test twice because the first time there is almost always
	// a considerably higher latency
	i64 diff = RunIter();
	if (diff > maxLatencyDiff) {
		diff = RunIter();
	}

	EXPECT_LE(diff, maxLatencyDiff);

	// std::cout << "STD  :" << dt1 << std::endl;
	// std::cout << "Win32:" << dt2 << std::endl;
	// std::cout << "Diff :" << diff << std::endl;
}
