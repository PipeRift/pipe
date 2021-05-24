// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"

#include <stddef.h>

#include <type_traits>


/** SIZE SELECTORS */

namespace Rift
{
	template <typename T>
	concept IsVoid = std::is_void_v<T>;

	template <typename One, typename Other>
	concept IsSame = std::is_same_v<One, Other>;

	template <typename Child, typename Base, bool bIncludeSame = true>
	concept Derived = (bIncludeSame && IsSame<Child, Base>) || std::is_base_of_v<Base, Child>;

	template <typename From, typename To>
	concept Convertible = std::is_same_v<From, To> || std::is_convertible_v<From, To>;

	template <typename T>
	concept Integral = std::is_integral_v<T>;

	template <typename T>
	concept IsMoveConstructible = std::is_move_constructible_v<T>;
	template <typename T>
	concept IsMoveAssignable = std::is_move_constructible_v<T>;

	template <bool B, typename T = void>
	using EnableIf = std::enable_if<B, T>;

	template <bool B, typename T = void>
	using EnableIfT = std::enable_if_t<B, T>;

	template <typename T, sizet size>
	concept IsSmaller = sizeof(T) < size;

	template <typename T, sizet size>
	concept IsBigger = sizeof(T) > size;

	template <bool Expression, typename True, typename False>
	using SelectType = std::conditional<Expression, True, False>;

	template <typename T>
	concept ShouldPassByValue = sizeof(T) <= sizeof(sizet) && std::is_copy_constructible_v<T>;

	template <bool useT, typename T, typename F>
	using Select = typename std::conditional<useT, T, F>::type;

	template <typename T>
	struct HasItemType
	{
	private:
		template <typename V>
		static void Impl(decltype(typename V::ItemType(), int()));
		template <typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template <typename T>
	struct HasKeyType
	{
	private:
		template <typename V>
		static void Impl(decltype(typename V::KeyType(), int()));
		template <typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template <typename T>
	struct HasValueType
	{
	private:
		template <typename V>
		static void Impl(decltype(typename V::ValueType(), int()));
		template <typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};
}    // namespace Rift

#define RIFT_DECLARE_IS_TRIVIAL(T, isTrivial)                                                \
	namespace std                                                                                                            \
	{                                                                                                                        \
		template <>                                                                                                          \
		struct is_trivial<T> : public std::integral_constant<bool, isTrivial>                \
		{};                                                                                                                  \
		template <>                                                                                                          \
		struct is_trivial<const T> : public std::integral_constant<bool, isTrivial>          \
		{};                                                                                                                  \
		template <>                                                                                                          \
		struct is_trivial<volatile T> : public std::integral_constant<bool, isTrivial>       \
		{};                                                                                                                  \
		template <>                                                                                                          \
		struct is_trivial<const volatile T> : public std::integral_constant<bool, isTrivial> \
		{};                                                                                                                  \
	}
