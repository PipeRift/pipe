// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/STDFormat.h"


namespace p::internal
{
	PIPE_API void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, const char* text);

	inline void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, const std::string& text)
	{
		// Ensure string is not destroyed while using it
		FailedCheckError(expr, file, line, text.c_str());
	}
}    // namespace p::internal


#define EnsureImpl(capture, always, expression, text)                         \
	(P_LIKELY(!!(expression)) || ([capture]() {                               \
		static bool executed = false;                                         \
		if (always || !executed)                                              \
		{                                                                     \
			executed = true;                                                  \
			return true;                                                      \
		}                                                                     \
		return false;                                                         \
	}()) && ([capture]() {                                                    \
		p::internal::FailedCheckError(#expression, __FILE__, __LINE__, text); \
		P_DEBUG_PLATFORM_BREAK();                                             \
		return false;                                                         \
	}()))

#define Ensure(expression) EnsureImpl(, false, expression, "")
#define EnsureMsg(expression, msg, ...) \
	EnsureImpl(&, false, expression, std::format(msg, ##__VA_ARGS__))

#ifndef Check
	#if P_RELEASE
		#define CheckImpl(expression, ...)
	#else
		#define CheckImpl(expression, text)                                           \
			if (!(expression)) [[unlikely]]                                           \
			{                                                                         \
				p::internal::FailedCheckError(#expression, __FILE__, __LINE__, text); \
				P_DEBUG_PLATFORM_BREAK();                                             \
			}
	#endif

	#define Check(expression) CheckImpl(expression, "")
	#define CheckMsg(expression, msg, ...) CheckImpl(expression, std::format(msg, ##__VA_ARGS__))
#endif


#define NotImplemented CheckMsg(false, "Not Implemented")