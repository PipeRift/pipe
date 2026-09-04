// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "PipeStrings.h"

#include <mutex>


namespace p
{
	namespace Terminal
	{
		// Foreground colors
		inline constexpr const char* ColorReset    = "\033[0m";
		inline constexpr const char* Black         = "\033[30m";
		inline constexpr const char* Red           = "\033[31m";
		inline constexpr const char* Green         = "\033[32m";
		inline constexpr const char* Yellow        = "\033[33m";
		inline constexpr const char* Blue          = "\033[34m";
		inline constexpr const char* Magenta       = "\033[35m";
		inline constexpr const char* Cyan          = "\033[36m";
		inline constexpr const char* White         = "\033[37m";
		inline constexpr const char* BrightBlack   = "\033[90m";
		inline constexpr const char* BrightRed     = "\033[91m";
		inline constexpr const char* BrightGreen   = "\033[92m";
		inline constexpr const char* BrightYellow  = "\033[93m";
		inline constexpr const char* BrightBlue    = "\033[94m";
		inline constexpr const char* BrightMagenta = "\033[95m";
		inline constexpr const char* BrightCyan    = "\033[96m";
		inline constexpr const char* BrightWhite   = "\033[97m";

		// Background colors
		inline constexpr const char* BgBlack         = "\033[40m";
		inline constexpr const char* BgRed           = "\033[41m";
		inline constexpr const char* BgGreen         = "\033[42m";
		inline constexpr const char* BgYellow        = "\033[43m";
		inline constexpr const char* BgBlue          = "\033[44m";
		inline constexpr const char* BgMagenta       = "\033[45m";
		inline constexpr const char* BgCyan          = "\033[46m";
		inline constexpr const char* BgWhite         = "\033[47m";
		inline constexpr const char* BgBrightBlack   = "\033[100m";
		inline constexpr const char* BgBrightRed     = "\033[101m";
		inline constexpr const char* BgBrightGreen   = "\033[102m";
		inline constexpr const char* BgBrightYellow  = "\033[103m";
		inline constexpr const char* BgBrightBlue    = "\033[104m";
		inline constexpr const char* BgBrightMagenta = "\033[105m";
		inline constexpr const char* BgBrightCyan    = "\033[106m";
		inline constexpr const char* BgBrightWhite   = "\033[107m";
	}    // namespace Terminal


	struct Logger
	{
		std::function<void(StringView)> infoCallback;
		std::function<void(StringView)> warningCallback;
		std::function<void(StringView)> errorCallback;
	};

	P_API void InitLog(Logger* logger = nullptr);
	P_API void ShutdownLog();

	P_API void Info(StringView msg);
	P_API void Warning(StringView msg);
	P_API void Error(StringView msg);

	template<typename... Args>
	void Info(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Info(Format(format, std::forward<Args>(args)...));
		}
	}

	template<typename... Args>
	void Warning(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Warning(Format(format, std::forward<Args>(args)...));
		}
	}

	template<typename... Args>
	void Error(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Error(Format(format, std::forward<Args>(args)...));
		}
	}
};    // namespace p
