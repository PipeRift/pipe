// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/TypeTraits.h"

#include <format>


// Support for std::format of pointers
template<typename T>
requires(!p::IsVoid<T> && !p::IsChar<T>)
struct std::formatter<T*> : public std::formatter<const void*>
{
	template<typename FormatContext>
	auto format(T* ptr, FormatContext& ctx) const
	{
		return formatter<const void*>::format(ptr, ctx);
	}
};
