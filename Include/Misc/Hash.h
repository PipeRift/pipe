// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Platform/Platform.h"


namespace Rift
{
	template <typename T>
	struct Hash : robin_hood::hash<T>
	{
		sizet operator()(T const& obj)
		{
			return robin_hood::hash<T>::operator()(obj);
		}
	};

	inline sizet HashBytes(void const* ptr, sizet const len) noexcept
	{
		return robin_hood::hash_bytes(ptr, len);
	}
}	 // namespace Rift
