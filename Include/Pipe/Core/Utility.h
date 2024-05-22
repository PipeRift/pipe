// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/TypeTraits.h"

#include <utility>


namespace p
{
	template<typename T>
	constexpr void Swap(T& a, T& b) noexcept
	{
		std::swap(a, b);
	}

	// Forward arg as movable
	template<typename T>
	constexpr RemoveReference<T>&& Move(T&& arg) noexcept
	{
		return static_cast<RemoveReference<T>&&>(arg);
	}

	// Forward an lvalue as either an lvalue or an rvalue
	template<typename T>
	constexpr T&& Forward(RemoveReference<T>& arg)
	{
		return static_cast<T&&>(arg);
	}

	// Forward an rvalue as an rvalue
	template<typename T>
	constexpr T&& Forward(RemoveReference<T>&& arg)
	{
		static_assert(!std::is_lvalue_reference_v<T>, "Bad Forward call");
		return static_cast<T&&>(arg);
	}

	template<typename T, typename OtherT = T>
	constexpr T Exchange(T& value, OtherT&& newValue) noexcept(
	    IsMoveConstructible<T>&& IsAssignable<T&, OtherT>)
	{
		// assign _New_val to _Val, return previous _Val
		T oldValue = p::Forward<T>(value);
		value      = p::Forward<OtherT>(newValue);
		return oldValue;
	}

	[[noreturn]] inline void Unreachable()
	{
#if defined(__GNUC__)
		__builtin_unreachable();
#elif defined(__has_builtin)
	#if __has_builtin(__builtin_unreachable)
		__builtin_unreachable();
	#endif
#elif defined(_MSC_VER)
		__assume(false);
#endif
	}

	struct Undefined
	{
		explicit Undefined() = default;
	};

	template<typename Predicate>
	class ReversePredicate
	{
		const Predicate& predicate;

	public:
		ReversePredicate(const Predicate& predicate) : predicate(predicate) {}

		template<typename T>
		bool operator()(T&& A, T&& B) const
		{
			return predicate(p::Forward<T>(B), p::Forward<T>(A));
		}
	};
}    // namespace p
