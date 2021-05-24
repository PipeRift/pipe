// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Log.h"
#include "Platform/Platform.h"

template <typename RetType = void, typename InnerType>
RetType RunCheckCallback(InnerType&& callback)
{
	return callback();
}

#define Ensure_Impl(capture, always, expression, ...)                              \
	(LIKELY(!!(expression)) || (RunCheckCallback<bool>([capture]() {               \
		static bool executed = false;                                              \
		if (!executed || always)                                                   \
		{                                                                          \
			executed = true;                                                       \
			Log::FailedCheckError(#expression, __FILE__, __LINE__, ##__VA_ARGS__); \
			return true;                                                           \
		}                                                                          \
		return false;                                                              \
	}) && ([]() {                                                                  \
		PLATFORM_BREAK();                                                          \
		return false;                                                              \
	}())))


#define Ensure(expression) Ensure_Impl(, false, expression, "")
#define EnsureMsg(expression, format, ...) Ensure_Impl(&, false, expression, format, ##__VA_ARGS__)


#ifndef Check
#	define Check(expr)           \
		if (!(expr)) [[unlikely]] \
		{                         \
			PLATFORM_BREAK();     \
		}
#endif
