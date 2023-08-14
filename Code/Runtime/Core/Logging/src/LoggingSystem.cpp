#include "LoggingSystem.h"

#include <cstdarg>
#include <format>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace CKE {

	void LoggingSystem::Initialize() {
		m_LogEntries.reserve(1000);
	}

	void LoggingSystem::Shutdown() {
		m_LogEntries.clear();
	}

	void LoggingSystem::AddLogEntryVA(LogLevel level, LogChannel channel, char const* format, va_list vaList) {
		constexpr u32 MAX_MSG_SIZE = 4096;
		static char   s_LogBuffer[MAX_MSG_SIZE];
		FormatVA(s_LogBuffer, MAX_MSG_SIZE, format, vaList);

		printf("%s", s_LogBuffer);

		LogEntry entry{};
		entry.m_Channel = channel;
		entry.m_Level = level;
		entry.m_Message = String{s_LogBuffer};
		m_LogEntries.emplace_back(entry);
	}

	void LoggingSystem::Format(char* pBuffer, u32 bufferSize, char const* format, ...) {
		va_list vaList;
		va_start(vaList, format);
		FormatVA(pBuffer, bufferSize, format, vaList);
		va_end(vaList);
	}

	void LoggingSystem::FormatVA(char* pBuffer, u32 bufferSize, char const* format, va_list vaList) {
		vsnprintf(pBuffer, bufferSize, format, vaList);
	}

	void LoggingSystem::OutputLogEntry(LogEntry const& entry) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		// Get data about current time
		SetConsoleTextAttribute(hConsole, 8);
		std::cout << entry.m_TimeStamp;
		SetConsoleTextAttribute(hConsole, 7);
		std::cout << " | ";
		SetConsoleTextAttribute(hConsole, 2);
		std::cout << s_LogLevelLabels[static_cast<i32>(entry.m_Level)];
		SetConsoleTextAttribute(hConsole, 7);
		std::cout << " | ";
		SetConsoleTextAttribute(hConsole, 3);
		std::cout << s_LogChannelsLabels[static_cast<i32>(entry.m_Channel)];
		SetConsoleTextAttribute(hConsole, 7);
		std::cout << " | ";
		std::cout << entry.m_Message;
	}

	void LoggingSystem::AddLogEntry(LogLevel level, LogChannel channel, char const* format, ...) {
		va_list vaList;
		va_start(vaList, format);
		AddLogEntryVA(level, channel, format, vaList);
		va_end(vaList);
	}
}
