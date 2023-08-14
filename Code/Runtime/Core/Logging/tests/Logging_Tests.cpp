#include "CookieKat/Core/Logging/LoggingSystem.h"
#include <gtest/gtest.h>

using namespace CKE;

class LoggingSystemTest : public testing::Test
{
protected:
	void SetUp() override {
		g_LoggingSystem.Initialize();
		// To be able to test the console output, we redirect it to our own temporary buffer
		m_OriginalConsoleBuffer = std::cout.rdbuf();
		std::cout.rdbuf(m_ConsoleBuffer.rdbuf());
	}

	void TearDown() override {
		std::cout.rdbuf(m_OriginalConsoleBuffer);
		g_LoggingSystem.Shutdown();
	}

	std::stringstream m_ConsoleBuffer;

private:
	std::streambuf*   m_OriginalConsoleBuffer = nullptr;
};

TEST_F(LoggingSystemTest, Log) {
	g_LoggingSystem.Log(LogLevel::Warning, LogChannel::Assets, "Number: {}, Str: {}", 42, "Pepe");
	String consoleOutput = m_ConsoleBuffer.str();
	ASSERT_NE(consoleOutput.find("Number: 42, Str: Pepe"), String::npos);
	ASSERT_NE(consoleOutput.find("Warning"), String::npos);
	ASSERT_NE(consoleOutput.find("Assets"), String::npos);
}

TEST_F(LoggingSystemTest, Simple) {
	g_LoggingSystem.Simple("Number: {}, Str: {}", 42, "Pepe");
	String consoleOutput = m_ConsoleBuffer.str();
	ASSERT_NE(consoleOutput.find("Number: 42, Str: Pepe"), String::npos);
}
