// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Log.h"


#ifndef Ensure
#	define Ensure(condition, ...)     \
		if (!(condition)) [[unlikely]] \
		{                              \
			Log::Error(__VA_ARGS__);   \
			PLATFORM_BREAK();          \
		}
#	define EnsureIf(condition, ...) Ensure(condition, __VA_ARGS__) else
#endif

#ifndef Check
#	define Check(expr)         \
		if (!(expr)) [[unlikely]]    \
		{                            \
			PLATFORM_BREAK();        \
		}
#endif
