#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"
#include <format>
#include <iostream>
#include <chrono>

namespace CKE {
	// Macros to easily define logging levels and channels and their required metadata
	//-----------------------------------------------------------------------------

#define CKE_LOGGING_DEF_LEVELS(DEF) \
	DEF(Info) \
	DEF(Warning) \
	DEF(Error) \
	DEF(Debug) \
	DEF(Fatal)

	// All of the available log channels
	// Can define more if needed
#define CKE_LOGGING_DEF_CHANNEL(DEF) \
	DEF(Assets) \
	DEF(Resources) \
	DEF(Rendering) \
	DEF(ECS) \
	DEF(Core) \
	DEF(Game)

	//-----------------------------------------------------------------------------

#define CKE_DEFINE_LOG_ENUM(d) d,

	enum class LogLevel : u8 { CKE_LOGGING_DEF_LEVELS(CKE_DEFINE_LOG_ENUM) };

	enum class LogChannel : u8 { CKE_LOGGING_DEF_CHANNEL(CKE_DEFINE_LOG_ENUM) };

#undef CKE_DEFINE_LOG_ENUM

	//-----------------------------------------------------------------------------

	// Standard logging entry data
	struct LogEntry
	{
		LogLevel                                                                                                  m_Level;
		LogChannel                                                                                                m_Channel;
		String                                                                                                    m_Message;
		std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<1, 1000>>> m_TimeStamp; // Pls
	};

	// General purpose logging system of the engine
	class LoggingSystem
	{
	public:
		// Lifetime
		//-----------------------------------------------------------------------------

		void Initialize();
		void Shutdown();

		// Templated API
		//-----------------------------------------------------------------------------

		// Logs a message directly without appending any extra information and without any tracking.
		// Utilizes std::vformat to format the supplied message.
		//
		// Example:
		//   Simple("Number {}", 42);
		//
		// Which outputs:
		//   Number 42 
		template <typename... Args>
		void Simple(char const* format, Args&&... args);

		// Logs a message using the standard format alongside the provided one.
		// Utilizes std::vformat to format the supplied message.
		//
		// Example:
		//   Log(LogLevel::Info, LogChannel::Assets, "Number {}", 42);
		//
		// Which outputs:
		//   24:60:60.000 | Info | Assets | Number 42 
		template <typename... Args>
		void Log(LogLevel level, LogChannel channel, char const* format, Args&&... args);

		// C Based API
		//-----------------------------------------------------------------------------

		void AddLogEntry(LogLevel level, LogChannel channel, char const* format, ...);
		void AddLogEntryVA(LogLevel level, LogChannel channel, char const* format, va_list vaList);

		// Format a message using vsnprintf
		void Format(char* pBuffer, u32 bufferSize, char const* format, ...);
		void FormatVA(char* pBuffer, u32 bufferSize, char const* format, va_list vaList);

	private:
		// Outputs a log entry to the console using its standard format
		void OutputLogEntry(LogEntry const& entry);

	private:
		Vector<LogEntry> m_LogEntries{}; // All log messages in the current execution

#define CKE_LOG_DEFINE_STR(x) #x,
		constexpr static char const* const s_LogLevelLabels[] = {
			CKE_LOGGING_DEF_LEVELS(CKE_LOG_DEFINE_STR)
		};
		constexpr static char const* const s_LogChannelsLabels[] = {
			CKE_LOGGING_DEF_CHANNEL(CKE_LOG_DEFINE_STR)
		};
#undef CKE_LOG_DEFINE_STR
	};

	// Global logging system of the engine
	inline LoggingSystem g_LoggingSystem{};
}

// Template Implementation
//-----------------------------------------------------------------------------

namespace CKE {
	template <typename... Args>
	void LoggingSystem::Simple(char const* format, Args&&... args) {
		std::cout << std::vformat(format, std::make_format_args(args...));
	}

	template <typename... Args>
	void LoggingSystem::Log(LogLevel  level, LogChannel channel, char const* format,
	                        Args&&... args) {
		LogEntry entry{};
		entry.m_TimeStamp = std::chrono::floor<std::chrono::milliseconds>(
			std::chrono::system_clock::now());
		entry.m_Channel = channel;
		entry.m_Level = level;
		entry.m_Message = std::vformat(format, std::make_format_args(args...));
		m_LogEntries.emplace_back(entry);

		OutputLogEntry(entry);

		if (level == LogLevel::Fatal) {
			CKE_UNREACHABLE_CODE();
		}
	}
}
