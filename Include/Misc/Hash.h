// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"


namespace Rift
{
	template <typename T>
	struct Hash : robin_hood::hash<T>
	{
		sizet operator()(const T& obj) const
		{
			return robin_hood::hash<T>::operator()(obj);
		}
	};

	inline CORE_API sizet HashBytes(void const* ptr, sizet const len) noexcept
	{
		return robin_hood::hash_bytes(ptr, len);
	}

	// FNV String hash
	// Use offset and prime based on the architecture (64bit or 32bit)
	// http://www.isthe.com/chongo/tech/comp/fnv/index.html
	template <typename CharType = TChar>
	inline CORE_API constexpr sizet GetStringHash(const CharType* str)
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
	template <typename CharType = TChar>
	inline CORE_API constexpr sizet GetStringHash(const CharType* str, sizet size)
	{
		sizet i = 0;
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
}    // namespace Rift
