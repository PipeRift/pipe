// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <magic_enum.hpp>


namespace Rift::EnumOperators
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

	template<typename E>
	constexpr bool HasAllFlags(E value, E flags) noexcept requires(IsEnum<E>)
	{
		return (value & flags) == flags;
	}

	template<typename E>
	constexpr bool HasAnyFlags(E value, E flags) noexcept requires(IsEnum<E>)
	{
		return *(value & flags) != 0;
	}

	template<typename E>
	constexpr bool HasFlag(E value, E flag) noexcept requires(IsEnum<E>)
	{
		return HasAllFlags(value, flag);
	}

	template<typename E>
	void AddFlags(E& value, E flags) noexcept requires(IsEnum<E>)
	{
		value |= flags;
	}

	template<typename E>
	void RemoveFlags(E& value, E flags) noexcept requires(IsEnum<E>)
	{
		value &= ~flags;
	}
}    // namespace Rift::EnumOperators
