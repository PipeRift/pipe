// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Utility.h"


namespace p
{
	template<typename T = void>
	struct TGreater
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A > B;
		}
	};

	template<>
	struct TGreater<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) > Forward<U>(B);
		}
	};

	template<typename T = void>
	struct TGreaterEqual
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A >= B;
		}
	};

	template<>
	struct TGreaterEqual<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) >= Forward<U>(B);
		}
	};
}    // namespace p