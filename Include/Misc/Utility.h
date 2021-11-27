// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <type_traits>


namespace Rift
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
}    // namespace Rift
