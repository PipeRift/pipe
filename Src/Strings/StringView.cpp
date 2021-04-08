// Copyright 2015-2021 Piperift - All rights reserved

#include "Strings/StringView.h"

#include <charconv>


namespace Rift::Strings
{
	TOptional<u32> ToU32(StringView str)
	{
		u32 val;
		if (std::from_chars(str.begin(), str.end(), val).ec != std::errc())
		{
			return val;
		}
		return {};
	}

	TOptional<i32> ToI32(StringView str)
	{
		i32 val;
		if (std::from_chars(str.begin(), str.end(), val).ec != std::errc())
		{
			return val;
		}
		return {};
	}
}    // namespace Rift::Strings
