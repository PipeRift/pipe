// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"

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


	template <typename T>
	concept IsMoveConstructible = std::is_move_constructible_v<T>;
	template <typename T>
	concept IsMoveAssignable = std::is_move_constructible_v<T>;

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
		static void Impl(decltype(typename V::ItemType(), int()));
		template <typename V>
		static bool Impl(char);

	public:
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};


	template <typename Predicate>
	class ReversePredicate
	{
		const Predicate& predicate;

	public:
		ReversePredicate(const Predicate& predicate) : predicate(predicate) {}

		template <typename T>
		bool operator()(T&& A, T&& B) const
		{
			return predicate(Forward<T>(B), Forward<T>(A));
		}
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
