// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include <stddef.h>

#include <type_traits>


/** SIZE SELECTORS */

namespace Rift
{
	template <typename Child, typename Base, bool bIncludeSame = true>
	concept Derived =
	    (bIncludeSame && std::is_same_v<Child, Base>) || std::is_base_of_v<Base, Child>;

	template <typename From, typename To>
	concept Convertible = std::is_same_v<From, To> || std::is_convertible_v<From, To>;

	template <bool B, typename T = void>
	using EnableIf = std::enable_if<B, T>;

	template <bool B, typename T = void>
	using EnableIfT = std::enable_if_t<B, T>;

	template <typename T, sizet size>
	struct IsSmallerType : std::integral_constant<bool, (sizeof(T) <= size)>
	{};

	template <typename T, sizet size>
	struct IsBiggerType : std::integral_constant<bool, (sizeof(T) > size)>
	{};

	template <bool Expression, typename True, typename False>
	using SelectType = std::conditional<Expression, True, False>;


#define EnableIfSmallerType(size) typename = EnableIf<IsSmallerType<T, size>::value>
#define EnableIfNotSmallerType(size) typename = EnableIf<!IsSmallerType<T, size>::value>

#define EnableIfBiggerType(size) typename = EnableIf<IsBiggerType<T, size>::value>
#define EnableIfNotBiggerType(size) typename = EnableIf<!IsBiggerType<T, size>::value>

#define EnableIfPassByValue(T) \
	typename =                 \
	    EnableIf < IsSmallerType<T, sizeof(sizet)>::value && std::is_copy_constructible<T>::type >
#define EnableIfNotPassByValue(T) \
	typename =                    \
	    EnableIf<!(IsSmallerType<T, sizeof(sizet)>::value && std::is_copy_constructible<T>::type)>

#define EnableIfAll typename = void


	template <typename T>
	struct HasItemType
	{
	private:
		template <typename V>
		static void impl(decltype(typename V::ItemType(), int()));
		template <typename V>
		static bool impl(char);

	public:
		static const bool value = std::is_void<decltype(impl<T>(0))>::value;
	};

}    // namespace Rift

#define RIFT_DECLARE_IS_POD(T, isPod)                                                \
	namespace std                                                                    \
	{                                                                                \
		template <>                                                                  \
		struct is_pod<T> : public std::integral_constant<bool, isPod>                \
		{};                                                                          \
		template <>                                                                  \
		struct is_pod<const T> : public std::integral_constant<bool, isPod>          \
		{};                                                                          \
		template <>                                                                  \
		struct is_pod<volatile T> : public std::integral_constant<bool, isPod>       \
		{};                                                                          \
		template <>                                                                  \
		struct is_pod<const volatile T> : public std::integral_constant<bool, isPod> \
		{};                                                                          \
	}
