// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Extern/magic_enum.hpp"


namespace p
{
	template<Integral I>
	constexpr bool HasAllFlags(I value, I flags) noexcept
	{
		return (value & flags) == flags;
	}

	template<Integral I>
	constexpr bool HasAnyFlags(I value, I flags) noexcept
	{
		return (value & flags) != 0;
	}

	template<Integral I>
	constexpr bool HasFlag(I value, I flag) noexcept
	{
		return HasAllFlags(value, flag);
	}

	namespace EnumOperators
	{
		// Bitwise operators: ~, |, &, ^, |=, &=, ^=
		using namespace magic_enum::bitwise_operators;

		template<IsEnum E>
		constexpr UnderlyingType<E> operator*(E value) noexcept
		{
			return static_cast<UnderlyingType<E>>(value);
		}

		template<IsEnum E>
		constexpr E operator|(E lhs, E rhs) noexcept
		{
			return static_cast<E>(
			    static_cast<UnderlyingType<E>>(lhs) | static_cast<UnderlyingType<E>>(rhs));
		}
	}    // namespace EnumOperators

	template<IsEnum E>
	constexpr bool HasAllFlags(E value, E flags) noexcept
	{
		return (static_cast<UnderlyingType<E>>(value) & static_cast<UnderlyingType<E>>(flags))
		    == static_cast<UnderlyingType<E>>(flags);
	}

	template<IsEnum E>
	constexpr bool HasAnyFlags(E value, E flags) noexcept
	{
		return (static_cast<UnderlyingType<E>>(value) & static_cast<UnderlyingType<E>>(flags)) != 0;
	}

	template<IsEnum E, typename R = UnderlyingType<E>>
	constexpr bool HasAllFlags(R value, E flags) noexcept
	{
		return (static_cast<R>(value) & static_cast<UnderlyingType<E>>(flags))
		    == static_cast<UnderlyingType<E>>(flags);
	}

	template<IsEnum E, typename R = UnderlyingType<E>>
	constexpr bool HasAnyFlags(R value, E flags) noexcept
	{
		return (static_cast<R>(value) & static_cast<UnderlyingType<E>>(flags)) != 0;
	}

	template<IsEnum E>
	constexpr bool HasFlag(E value, E flag) noexcept
	{
		return HasAllFlags(value, flag);
	}

	template<IsEnum E, typename R = UnderlyingType<E>>
	constexpr bool HasFlag(R value, E flag) noexcept
	{
		return HasAllFlags(value, flag);
	}

	template<IsEnum E>
	void AddFlags(E& value, E flags) noexcept
	{
		auto rawValue = static_cast<UnderlyingType<E>>(value);
		rawValue |= static_cast<UnderlyingType<E>>(flags);
		value = static_cast<E>(rawValue);
	}

	template<IsEnum E>
	void RemoveFlags(E& value, E flags) noexcept
	{
		auto rawValue = static_cast<UnderlyingType<E>>(value);
		rawValue &= ~static_cast<UnderlyingType<E>>(flags);
		value = static_cast<E>(rawValue);
	}
}    // namespace p

#define P_DEFINE_FLAG_OPERATORS(Type)                       \
	constexpr p::UnderlyingType<Type> operator*(Type value) \
	{                                                       \
		return static_cast<p::UnderlyingType<Type>>(value); \
	}                                                       \
	constexpr Type operator|(Type lhs, Type rhs)            \
	{                                                       \
		return static_cast<Type>(*lhs | *rhs);              \
	}
