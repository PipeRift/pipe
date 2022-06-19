// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/StringView.h"

#include <charconv>


namespace p::core::Strings
{
	TOptional<u32> ToU32(StringView str)
	{
		u32 val;
		if (std::from_chars(str.data(), str.data() + str.size(), val).ec != std::errc())
		{
			return val;
		}
		return {};
	}

	TOptional<i32> ToI32(StringView str)
	{
		i32 val;
		if (std::from_chars(str.data(), str.data() + str.size(), val).ec != std::errc())
		{
			return val;
		}
		return {};
	}
}    // namespace p::core::Strings