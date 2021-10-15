// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Files/STDFileSystem.h"
#include "Strings/String.h"

#include <mutex>


namespace Rift::Log
{
	CORE_API void Init(Path logPath = {});
	CORE_API void Shutdown();

	void CORE_API Info(StringView msg);
	void CORE_API Warning(StringView msg);
	void CORE_API Error(StringView msg);

	template <typename... Args>
	void Info(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Info(Strings::Format(format, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	void Warning(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Warning(Strings::Format(format, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	void Error(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Error(Strings::Format(format, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, StringView format, Args... args)
	{
		if (!format.empty())
		{
			String newFormat =
			    Strings::Format("{} \n(Failed check \"{}\" at {}:{})", format, expr, file, line);
			Error(Strings::Format(newFormat, std::forward<Args>(args)...));
			return;
		}
		Error(Strings::Format("Failed check \"{}\" at {}:{}", expr, file, line));
	}
};    // namespace Rift::Log
