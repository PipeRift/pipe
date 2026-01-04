// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"

#include <cassert>
#include <utility>


namespace p
{
	template<typename... Type>
	struct TTypeList
	{
		using type = TTypeList;

		static constexpr auto size = sizeof...(Type);
	};

	template<typename... Type, typename... Other>
	constexpr TTypeList<Type..., Other...> operator+(TTypeList<Type...>, TTypeList<Other...>)
	{
		return {};
	}

	template<std::size_t, typename>
	struct TTypeListIterator;

	template<std::size_t index, typename Type, typename... Other>
	struct TTypeListIterator<index, TTypeList<Type, Other...>>
	    : TTypeListIterator<index - 1u, TTypeList<Other...>>
	{};

	template<typename Type, typename... Other>
	struct TTypeListIterator<0u, TTypeList<Type, Other...>>
	{
		/*! @brief Searched type. */
		using type = Type;
	};

	template<typename... T>
	struct TJoinList;

	template<typename... ATypes, typename... BTypes>
	struct TJoinList<TTypeList<ATypes...>, TTypeList<BTypes...>>
	{
		using Type = TTypeList<ATypes..., BTypes...>;
	};

	template<typename... ATypes, typename... BTypes>
	struct TJoinList<TTypeList<ATypes...>, BTypes...>
	{
		using Type = TTypeList<ATypes..., BTypes...>;
	};

	template<typename... T>
	using JoinList = TJoinList<T...>;


	/**
	 * @brief Helper type.
	 * @tparam Index Index of the type to return.
	 * @tparam List Type list to search into.
	 */
	template<std::size_t index, typename List>
	using TTypeListIndex = typename TTypeListIterator<index, List>::type;


	namespace Internal
	{
		template<typename T, typename List>
		struct TTypeListContains;

		template<typename T, typename... Us>
		struct TTypeListContains<T, const TTypeList<Us...>>
		    : std::disjunction<std::is_same<T, Us>...>
		{};

		template<typename T, typename... Us>
		struct TTypeListContains<T, TTypeList<Us...>> : std::disjunction<std::is_same<T, Us>...>
		{};
	}    // namespace Internal

	template<typename List, typename T>
	constexpr bool ListContains()
	{
		return Internal::TTypeListContains<T, List>::value;
	}


	template<typename T1, typename T2>
	struct TPair
	{
		using FirstType  = T1;
		using SecondType = T2;

		T1 first;
		T2 second;


		constexpr explicit(
		    !IsImplicitlyDefaultConstructible<T1> || !IsImplicitlyDefaultConstructible<T2>) TPair()
		    requires(IsDefaultConstructible<T1> && IsDefaultConstructible<T2>)
		    : first(), second()
		{}

		constexpr explicit(!IsConvertible<const T1&, T1> || !IsConvertible<const T2&, T2>)
		    TPair(const T1& inFirst, const T2& inSecond)
		        requires(IsCopyConstructible<T1> && IsCopyConstructible<T2>)
		    : first(inFirst), second(inSecond)
		{}

		template<typename OtherT1, typename OtherT2>
		constexpr explicit(!IsConvertible<OtherT1, T1> || !IsConvertible<OtherT2, T2>)
		    TPair(OtherT1&& inFirst, OtherT2&& inSecond)
		        requires(IsConstructible<T1, OtherT1> && IsConstructible<T2, OtherT2>)
		    : first(p::Forward<OtherT1>(inFirst)), second(p::Forward<OtherT2>(inSecond))
		{}

		TPair(const TPair& other) = default;
		TPair(TPair&& other)      = default;

		template<typename OtherT1, typename OtherT2>
		constexpr explicit(!IsConvertible<const OtherT1&, T1> || !IsConvertible<const OtherT2&, T2>)
		    TPair(const TPair<OtherT1, OtherT2>& other)
		        requires(IsConstructible<T1, const OtherT1&> && IsConstructible<T2, const OtherT2&>)
		    : first(other.first), second(other.second)
		{}

		template<class OtherT1, class OtherT2>
		constexpr explicit(!IsConvertible<OtherT1, T1> && !IsConvertible<OtherT2, T2>)
		    TPair(TPair<OtherT1, OtherT2>&& other)
		        requires(IsConstructible<T1, OtherT1> && IsConstructible<T2, OtherT2>)
		    : first(p::Forward<OtherT1>(other.first)), second(p::Forward<OtherT2>(other.second))
		{}

		TPair& operator=(const volatile TPair&) = delete;

		constexpr TPair& operator=(const TPair& other)
		    requires(IsCopyAssignable<T1> && IsCopyAssignable<T2>)
		{
			first  = other.first;
			second = other.second;
			return *this;
		}

		constexpr TPair& operator=(TPair&& other)
		    requires(IsMoveAssignable<T1> && IsMoveAssignable<T2>)
		{
			first  = p::Forward<T1>(other.first);
			second = p::Forward<T2>(other.second);
			return *this;
		}

		template<class OtherT1, class OtherT2>
		constexpr TPair& operator=(const TPair<OtherT1, OtherT2>& other)
		    requires(!IsSame<TPair, TPair<OtherT1, OtherT2>> && IsAssignable<T1&, const OtherT1&>
		             && IsAssignable<T2&, const OtherT2&>)
		{
			first  = other.first;
			second = other.second;
			return *this;
		}

		template<class OtherT1, class OtherT2>
		constexpr TPair& operator=(TPair<OtherT1, OtherT2>&& other)
		    requires(!IsSame<TPair, TPair<OtherT1, OtherT2>> && IsAssignable<T1&, OtherT1>
		             && IsAssignable<T2&, OtherT2>)
		{
			first  = p::Forward<OtherT1>(other.first);
			second = p::Forward<OtherT2>(other.second);
			return *this;
		}

		constexpr void Swap(TPair& other)
		{
			if (this != &other)
			{
				p::Swap(first, other.first);      // intentional ADL
				p::Swap(second, other.second);    // intentional ADL
			}
		}

		// std::pair shares same memory footprint
		constexpr operator std::pair<T1, T2>&()
		{
			return *reinterpret_cast<std::pair<T1, T2>*>(this);
		}
		constexpr operator const std::pair<T1, T2>&() const
		{
			return *reinterpret_cast<const std::pair<T1, T2>*>(this);
		}
	};

	template<typename... T>
	using TTuple = std::tuple<T...>;


	namespace Internal
	{
		template<typename T, typename Tuple>
		struct TTupleContains;

		template<typename T, typename... Us>
		struct TTupleContains<T, const TTuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
		{};

		template<typename T, typename... Us>
		struct TTupleContains<T, TTuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
		{};
	}    // namespace Internal

	template<typename Tuple, typename T>
	constexpr bool TupleContains()
	{
		return Internal::TTupleContains<T, Tuple>::value;
	}
}    // namespace p
