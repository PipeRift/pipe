// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Files/STDFileSystem.h"
#include "Strings/String.h"

#include <mutex>


namespace Pipe::Log
{
	CORE_API void Init(Path logPath = {});
	CORE_API void Shutdown();

	void CORE_API Info(StringView msg);
	void CORE_API Warning(StringView msg);
	void CORE_API Error(StringView msg);

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
};    // namespace Pipe::Log
