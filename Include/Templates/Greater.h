// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Misc/Utility.h"


namespace Rift
{
	template <typename T = void>
	struct TGreater
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A > B;
		}
	};

	template <>
	struct TGreater<void>
	{
		template <typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) > Forward<U>(B);
		}
	};

	template <typename T = void>
	struct TGreaterEqual
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A >= B;
		}
	};

	template <>
	struct TGreaterEqual<void>
	{
		template <typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) >= Forward<U>(B);
		}
	};
}    // namespace Rift
