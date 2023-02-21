// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/TypeTraits.h"

#include <type_traits>
#include <utility>


namespace p::core
{
	template<typename T>
	constexpr void Swap(T& a, T& b) noexcept
	{
		std::swap(a, b);
	}

	// Forward arg as movable
	template<typename T>
	constexpr std::remove_reference_t<T>&& Move(T&& arg) noexcept
	{
		return static_cast<std::remove_reference_t<T>&&>(arg);
	}

	// Forward an lvalue as either an lvalue or an rvalue
	template<typename T>
	constexpr T&& Forward(std::remove_reference_t<T>& arg) noexcept
	{
		return static_cast<T&&>(arg);
	}

	// Forward an rvalue as an rvalue
	template<class T>
	constexpr T&& Forward(std::remove_reference_t<T>&& arg) noexcept
	{
		static_assert(!std::is_lvalue_reference_v<T>, "Bad Forward call");
		return static_cast<T&&>(arg);
	}

	template<typename T, typename OtherT = T>
	constexpr T Exchange(T& value, OtherT&& newValue) noexcept(
	    IsMoveConstructible<T>&& IsAssignable<T&, OtherT>)
	{
		// assign _New_val to _Val, return previous _Val
		T oldValue = Forward<T>(value);
		value      = Forward<OtherT>(newValue);
		return oldValue;
	}

	template<typename Predicate>
	class ReversePredicate
	{
		const Predicate& predicate;

	public:
		ReversePredicate(const Predicate& predicate) : predicate(predicate) {}

		template<typename T>
		bool operator()(T&& A, T&& B) const
		{
			return predicate(Forward<T>(B), Forward<T>(A));
		}
	};

	[[noreturn]] inline void Unreachable()
	{
#if __has_builtin(__builtin_unreachable)
		__builtin_unreachable();
#elif defined(_MSC_VER)    // MSVC
		std::abort();
#endif
	}
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
