// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "PipePlatform.h"

#include <functional>


namespace p
{
	P_API sizet HashBytes(void const* ptr, sizet const len);
	P_API sizet HashInt(u64 x);
	P_API sizet HashCombine(sizet a, sizet b);


	// FNV String hash
	// Use offset and prime based on the architecture (64bit or 32bit)
	// http://www.isthe.com/chongo/tech/comp/fnv/index.html
	template<typename CharType = char>
	inline P_API constexpr sizet GetStringHash(const CharType* str)
	{
		// 32/64 bit architecture switch
		if constexpr (sizeof(sizet) < 64)
		{
			u32 c = 0, result = 2166136261U;
			while ((c = u32(*str++)) != 0)
			{
				result = (result * 16777619) ^ c;
			}
			return static_cast<sizet>(result);
		}
		else
		{
			u64 c = 0, result = 14695981039346656037U;
			while ((c = u64(*str++)) != 0)
			{
				result = (result * 1099511628211) ^ c;
			}
			return static_cast<sizet>(result);
		}
	}

	template<typename CharType = char>
	inline P_API constexpr sizet GetStringHash(const CharType* str, sizet size)
	{
		// 32/64 bit architecture switch
		if constexpr (sizeof(sizet) < 64)
		{
			u32 c = 0, result = 2166136261U;
			for (sizet i = 0; i < size; ++i)
			{
				c      = u32(str[i]);
				result = (result * 16777619) ^ c;
			}
			return static_cast<sizet>(result);
		}
		else
		{
			u64 c = 0, result = 14695981039346656037U;
			for (sizet i = 0; i < size; ++i)
			{
				c      = u32(str[i]);
				result = (result * 1099511628211) ^ c;
			}
			return static_cast<sizet>(result);
		}
	}


	template<Number T>
	sizet GetHash(const T& value) noexcept requires(!IsEnum<T>)
	{
#if defined(__GNUC__) && !defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
		return HashInt(static_cast<u64>(value));
#if defined(__GNUC__) && !defined(__clang__)
	#pragma GCC diagnostic pop
#endif
	}

	template<IsEnum T>
	sizet GetHash(const T& value) noexcept
	{
		return GetHash(static_cast<UnderlyingType<T>>(value));
	}

	template<typename T>
	sizet GetHash(T* ptr) noexcept
	{
		return HashInt(reinterpret_cast<sizet>(ptr));
	}

	// clang-format off
	template<typename T>
	concept Hashable = requires(T a)
	{
		{ GetHash(a) } -> std::convertible_to<sizet>;
	};
	// clang-format on
}    // namespace p
