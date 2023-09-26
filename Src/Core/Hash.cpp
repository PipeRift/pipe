// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Char.h"

#include <robin_hood.h>


namespace p
{
	sizet HashBytes(void const* ptr, sizet const len) noexcept
	{
		return robin_hood::hash_bytes(ptr, len);
	}

	sizet HashInt(u64 x) noexcept
	{
		return robin_hood::hash_int(x);
	}
}    // namespace p