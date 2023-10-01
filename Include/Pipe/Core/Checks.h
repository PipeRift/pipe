// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"

#include <format>


namespace p::internal
{
	PIPE_API void FailedCheckError(const char* msg, sizet size);

	template<typename... Args>
	void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, const char* format, Args... args)
	{
		std::string msg;
		if (!format)
		{
			msg = std::format("Failed check \"{}\" at {}:{}", expr, file, line);
		}
		else
		{
			const std::string descriptiveFormat =
			    std::format("{} \n(Failed check \"{}\" at {}:{})", format, expr, file, line);
			msg =
			    std::vformat(descriptiveFormat, std::make_format_args(std::forward<Args>(args)...));
		}
		FailedCheckError(msg.c_str(), msg.size());
	}
}    // namespace p::internal


#define EnsureImpl(capture, always, expression, ...)                                   \
	(P_LIKELY(!!(expression)) || ([capture]() {                                        \
		static bool executed = false;                                                  \
		if (always || !executed)                                                       \
		{                                                                              \
			executed = true;                                                           \
			return true;                                                               \
		}                                                                              \
		return false;                                                                  \
	}()) && ([capture]() {                                                             \
		p::internal::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
		P_DEBUG_PLATFORM_BREAK();                                                      \
		return false;                                                                  \
	}()))

#define Ensure(expression) EnsureImpl(, false, expression, "")
#define EnsureMsg(expression, format, ...) EnsureImpl(&, false, expression, format, ##__VA_ARGS__)

#ifndef Check
	#if P_RELEASE
		#define CheckImpl(expression, ...)
	#else
		#define CheckImpl(expression, ...)                                                     \
			if (!(expression)) [[unlikely]]                                                    \
			{                                                                                  \
				p::internal::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
				P_DEBUG_PLATFORM_BREAK();                                                      \
			}
	#endif

	#define Check(expression) CheckImpl(expression, "")
	#define CheckMsg(expression, format, ...) CheckImpl(expression, format, ##__VA_ARGS__)
#endif


#define NotImplemented CheckMsg(false, "Not Implemented")