// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Platform/Platform.h"

#include <fmt/format.h>


namespace Rift::Checks
{

	template<typename RetType = void, typename InnerType>
	RetType RunCheckCallback(InnerType&& callback)
	{
		return callback();
	}

	CORE_API void FailedCheckError(const char* msg, sizet size);

	template<typename... Args>
	void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, const char* format, Args... args)
	{
		std::string msg;
		if (!format)
		{
			msg = fmt::format(fmt::runtime("Failed check \"{}\" at {}:{}"), expr, file, line);
		}
		else
		{
			const std::string descriptiveFormat = fmt::format(
			    fmt::runtime("{} \n(Failed check \"{}\" at {}:{})"), format, expr, file, line);
			msg = fmt::format(fmt::runtime(descriptiveFormat), std::forward<Args>(args)...);
		}
		FailedCheckError(msg.c_str(), msg.size());
	}

}    // namespace Rift::Checks


#define EnsureImpl(capture, always, expression, ...)                                        \
	(LIKELY(!!(expression)) || (Checks::RunCheckCallback<bool>([capture]() {                \
		static bool executed = false;                                                       \
		if (!executed || always)                                                            \
		{                                                                                   \
			executed = true;                                                                \
			Rift::Checks::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
			return true;                                                                    \
		}                                                                                   \
		return false;                                                                       \
	}) && ([capture]() {                                                                    \
		DEBUG_PLATFORM_BREAK();                                                             \
		return false;                                                                       \
	}())))


#define Ensure(expression) EnsureImpl(, false, expression, "")
#define EnsureMsg(expression, format, ...) EnsureImpl(&, false, expression, format, ##__VA_ARGS__)

#ifndef Check
#	if BUILD_RELEASE
#		define CheckImpl(expression, ...)
#	else
#		define CheckImpl(expression, ...)                                                      \
			if (!(expression)) [[unlikely]]                                                     \
			{                                                                                   \
				Rift::Checks::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
				DEBUG_PLATFORM_BREAK();                                                         \
			}
#	endif

#	define Check(expression) CheckImpl(expression, "")
#	define CheckMsg(expression, format, ...) CheckImpl(expression, format, ##__VA_ARGS__)
#endif
