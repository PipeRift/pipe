// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Platform.h"

#include <stddef.h>

#include <type_traits>


/** SIZE SELECTORS */

namespace pipe
{
	template<typename T>
	concept IsVoid = std::is_void_v<T>;

	template<typename One, typename Other>
	concept IsSame = std::is_same_v<One, Other>;

	template<typename Child, typename Base, bool bIncludeSame = true>
	concept Derived = (bIncludeSame && IsSame<Child, Base>) || std::is_base_of_v<Base, Child>;

	template<typename From, typename To>
	concept Convertible = std::is_same_v<From, To> || std::is_convertible_v<From, To>;

	template<typename T>
	concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;

	template<typename T>
	concept Integral = std::is_integral_v<T>;
	template<typename T>
	concept SignedIntegral = std::is_integral_v<T> && std::is_signed_v<T>;
	template<typename T>
	concept UnsignedIntegral = std::is_integral_v<T> && std::is_unsigned_v<T>;

	template<typename T>
	concept FloatingPoint = std::is_floating_point_v<T>;

	template<typename T>
	concept IsCopyConstructible = std::is_copy_constructible_v<T>;
	template<typename T>
	concept IsCopyAssignable = std::is_copy_assignable_v<T>;
	template<typename T>
	concept IsMoveConstructible = std::is_move_constructible_v<T>;
	template<typename T>
	concept IsMoveAssignable = std::is_move_assignable_v<T>;

	template<typename T>
	concept IsAbstract = std::is_abstract_v<T>;

	template<typename T>
	constexpr bool IsEmpty()
	{
		return std::is_empty_v<T>;
	}

	template<typename T, sizet size>
	concept IsSmaller = sizeof(T) < size;

	template<typename T, sizet size>
	concept IsBigger = sizeof(T) > size;

	template<typename T>
	concept IsEnum = std::is_enum_v<T>;

	template<typename T>
	concept ShouldPassByValue = sizeof(T) <= sizeof(sizet) && std::is_copy_constructible_v<T>;

	template<bool UseT, typename T, typename F>
	using Select = typename std::conditional<UseT, T, F>::type;


	template<typename T>
	struct HasTypeMember
	{
	private:
		template<typename V>
		static void Impl(decltype(typename V::Type(), int()));
		template<typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template<typename T>
	struct HasItemTypeMember
	{
	private:
		template<typename V>
		static void Impl(decltype(typename V::ItemType(), int()));
		template<typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template<typename T>
	struct HasKeyTypeMember
	{
	private:
		template<typename V>
		static void Impl(decltype(typename V::KeyType(), int()));
		template<typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template<typename T>
	struct HasValueTypeMember
	{
	private:
		template<typename V>
		static void Impl(decltype(typename V::ValueType(), int()));
		template<typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template<typename T>
	using UnderlyingType = typename std::underlying_type<T>::type;

	template<typename T>
	using Mut = std::remove_const_t<T>;
	template<typename T>
	using Const = std::add_const_t<T>;

	template<typename T>
	concept IsConst = std::is_const_v<T>;
	template<typename T>
	concept IsMutable = !std::is_const_v<T>;


	template<typename T, typename Reference>
	struct TCopyConst
	{
		using type = T;
	};
	template<typename T, typename Reference>
	struct TCopyConst<T, const Reference>
	{
		using type = const T;
	};
	template<typename T, typename Reference>
	using CopyConst = typename TCopyConst<T, Reference>::type;
}    // namespace pipe

#define RIFT_DECLARE_IS_TRIVIAL(T, isTrivial)                                                \
	namespace std                                                                            \
	{                                                                                        \
		template<>                                                                           \
		struct is_trivial<T> : public std::integral_constant<bool, isTrivial>                \
		{};                                                                                  \
		template<>                                                                           \
		struct is_trivial<const T> : public std::integral_constant<bool, isTrivial>          \
		{};                                                                                  \
		template<>                                                                           \
		struct is_trivial<volatile T> : public std::integral_constant<bool, isTrivial>       \
		{};                                                                                  \
		template<>                                                                           \
		struct is_trivial<const volatile T> : public std::integral_constant<bool, isTrivial> \
		{};                                                                                  \
	}
