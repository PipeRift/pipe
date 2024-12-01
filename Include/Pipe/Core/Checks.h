// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/STDFormat.h"
#include "PipePlatform.h"



namespace p::details
{
	PIPE_API void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, const char* text);

	inline void FailedCheckError(
	    const AnsiChar* expr, const AnsiChar* file, u32 line, const std::string& text)
	{
		// Ensure string is not destroyed while using it
		FailedCheckError(expr, file, line, text.c_str());
	}
}    // namespace p::details


#define P_EnsureImpl(capture, always, expression, text)                      \
	(P_LIKELY(!!(expression)) || ([capture]() {                              \
		static bool executed = false;                                        \
		if (always || !executed)                                             \
		{                                                                    \
			executed = true;                                                 \
			return true;                                                     \
		}                                                                    \
		return false;                                                        \
	}()) && ([capture]() {                                                   \
		p::details::FailedCheckError(#expression, __FILE__, __LINE__, text); \
		p::PlatformDebugBreak();                                             \
		return false;                                                        \
	}()))

#define P_Ensure(expression) P_EnsureImpl(, false, expression, "")
#define P_EnsureMsg(expression, msg, ...) \
	P_EnsureImpl(&, false, expression, std::format(msg, ##__VA_ARGS__))

#ifndef P_Check
	#if P_RELEASE
		#define P_CheckImpl(expression, ...)
	#else
		#define P_CheckImpl(expression, text)                                        \
			if (!(expression)) [[unlikely]]                                          \
			{                                                                        \
				p::details::FailedCheckError(#expression, __FILE__, __LINE__, text); \
				p::PlatformDebugBreak();                                             \
			}
	#endif

	#define P_Check(expression) P_CheckImpl(expression, "")
	#define P_CheckMsg(expression, msg, ...) \
		P_CheckImpl(expression, std::format(msg, ##__VA_ARGS__))
#endif


#define P_NotImplemented P_CheckMsg(false, "Not Implemented")