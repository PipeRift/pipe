// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/String.h"

#include <mutex>


namespace p
{
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
			Info(Strings::Format(format, std::forward<Args>(args)...));
		}
	}

	template<typename... Args>
	void Warning(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Warning(Strings::Format(format, std::forward<Args>(args)...));
		}
	}

	template<typename... Args>
	void Error(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Error(Strings::Format(format, std::forward<Args>(args)...));
		}
	}
};    // namespace p
