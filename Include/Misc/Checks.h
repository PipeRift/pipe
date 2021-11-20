// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Log.h"
#include "Platform/Platform.h"


template<typename RetType = void, typename InnerType>
RetType RunCheckCallback(InnerType&& callback)
{
	return callback();
}

#define EnsureImpl(capture, always, expression, ...)                                     \
	(LIKELY(!!(expression)) || (RunCheckCallback<bool>([capture]() {                     \
		static bool executed = false;                                                    \
		if (!executed || always)                                                         \
		{                                                                                \
			executed = true;                                                             \
			Rift::Log::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
			return true;                                                                 \
		}                                                                                \
		return false;                                                                    \
	}) && ([capture]() {                                                                 \
		DEBUG_PLATFORM_BREAK();                                                          \
		return false;                                                                    \
	}())))


#define Ensure(expression) EnsureImpl(, false, expression, "")
#define EnsureMsg(expression, format, ...) EnsureImpl(&, false, expression, format, ##__VA_ARGS__)

#ifndef Check
#	if BUILD_RELEASE
#		define CheckImpl(expression, ...)
#	else
#		define CheckImpl(expression, ...)                                                   \
			if (!(expression)) [[unlikely]]                                                  \
			{                                                                                \
				Rift::Log::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
				DEBUG_PLATFORM_BREAK();                                                      \
			}
#	endif

#	define Check(expression) CheckImpl(expression, "")
#	define CheckMsg(expression, format, ...) CheckImpl(expression, format, ##__VA_ARGS__)
#endif
