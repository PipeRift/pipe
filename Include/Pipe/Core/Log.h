// Copyright 2015-2024 Piperift - All rights reserved

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

	PIPE_API void InitLog(Logger* logger = nullptr);
	PIPE_API void ShutdownLog();

	PIPE_API void Info(StringView msg);
	PIPE_API void Warning(StringView msg);
	PIPE_API void Error(StringView msg);

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
