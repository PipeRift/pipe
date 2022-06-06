// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "TypeTraits.h"

#include <magic_enum.hpp>


namespace p::core
{
	namespace EnumOperators
	{
		// Bitwise operators: ~, |, &, ^, |=, &=, ^=
		using namespace magic_enum::bitwise_operators;

		template<typename E>
		constexpr UnderlyingType<E> operator*(E value) noexcept requires(IsEnum<E>)
		{
			return static_cast<UnderlyingType<E>>(value);
		}

		template<typename E>
		constexpr E operator|(E lhs, E rhs) noexcept requires(IsEnum<E>)
		{
			return static_cast<E>(
			    static_cast<UnderlyingType<E>>(lhs) | static_cast<UnderlyingType<E>>(rhs));
		}
	}    // namespace EnumOperators

	template<typename E>
	constexpr bool HasAllFlags(E value, E flags) noexcept requires(IsEnum<E>)
	{
		return (static_cast<UnderlyingType<E>>(value) & static_cast<UnderlyingType<E>>(flags))
		    == static_cast<UnderlyingType<E>>(flags);
	}

	template<typename E>
	constexpr bool HasAnyFlags(E value, E flags) noexcept requires(IsEnum<E>)
	{
		return (static_cast<UnderlyingType<E>>(value) & static_cast<UnderlyingType<E>>(flags)) != 0;
	}

	template<typename E>
	constexpr bool HasFlag(E value, E flag) noexcept requires(IsEnum<E>)
	{
		return HasAllFlags(value, flag);
	}

	template<typename E>
	void AddFlags(E& value, E flags) noexcept requires(IsEnum<E>)
	{
		auto rawValue = static_cast<UnderlyingType<E>>(value);
		rawValue |= static_cast<UnderlyingType<E>>(flags);
		value = static_cast<E>(rawValue);
	}

	template<typename E>
	void RemoveFlags(E& value, E flags) noexcept requires(IsEnum<E>)
	{
		auto rawValue = static_cast<UnderlyingType<E>>(value);
		rawValue &= ~static_cast<UnderlyingType<E>>(flags);
		value = static_cast<E>(rawValue);
	}
}    // namespace p::core

namespace p
{
	using namespace p::core;
}