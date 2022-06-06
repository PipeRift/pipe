// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/String.h"
#include "Files/STDFileSystem.h"

#include <mutex>


namespace p::Log
{
	PIPE_API void Init(Path logPath = {});
	PIPE_API void Shutdown();

	void PIPE_API Info(StringView msg);
	void PIPE_API Warning(StringView msg);
	void PIPE_API Error(StringView msg);

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
};    // namespace p::Log
