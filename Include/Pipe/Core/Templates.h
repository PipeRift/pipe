// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"

#include <cassert>
#include <utility>


namespace p
{
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


	namespace Detail
	{
		template<typename T, typename Tuple>
		struct TTupleContains;

		template<typename T, typename... Us>
		struct TTupleContains<T, const TTuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
		{};

		template<typename T, typename... Us>
		struct TTupleContains<T, TTuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
		{};
	}    // namespace Detail

	template<typename Tuple, typename T>
	constexpr bool TupleContains()
	{
		return Detail::TTupleContains<T, Tuple>::value;
	}


	template<typename... T>
	struct TTypeList;

	namespace Detail
	{
		template<typename T>
		struct TIsTypeList : FalseType
		{};
		template<typename... T>
		struct TIsTypeList<TTypeList<T...>> : TrueType
		{};

		template<typename ResultList, typename... Remaining>
		struct TTypeListUnique
		{
			using Type = ResultList;
		};

		template<typename ResultList, typename First, typename... Rest>
		struct TTypeListUnique<ResultList, First, Rest...>
		{
			using NextList = Select<ResultList::template Contains<First>(), ResultList,
			    typename ResultList::template Append<First>>;
			using Type     = typename TTypeListUnique<NextList, Rest...>::Type;
		};

		template<typename List, template<typename> typename Predicate>
		struct TFilter;

		template<template<typename> typename Predicate>
		struct TFilter<TTypeList<>, Predicate>
		{
			using Type = TTypeList<>;    // Empty result
		};

		template<typename Head, typename... Tail, template<typename> typename Predicate>
		struct TFilter<TTypeList<Head, Tail...>, Predicate>
		{
			// Filter the remaining types (Tail...)
			using FilteredTail = typename TFilter<TTypeList<Tail...>, Predicate>::Type;
			// Include Head if predicate is true; otherwise, use FilteredTail directly
			using Type = Select<Predicate<Head>::value,
			    typename FilteredTail::template Prepend<Head>,    // Prepend Head to FilteredTail
			    FilteredTail                                      // Skip Head
			    >;
		};
	};    // namespace Detail


	template<typename T>
	concept IsTypeList = Detail::TIsTypeList<T>::value;


	template<typename... T>
	struct TTypeList
	{
		using type = TTypeList;

		static constexpr auto size = sizeof...(T);


		template<typename U>
		static consteval bool Contains()
		{
			return (std::is_same_v<T, U> || ...);
		}

	private:
		template<typename... S>
		struct TAppend
		{
			using Type = TTypeList<T..., S...>;
		};
		template<typename... S>
		struct TAppend<TTypeList<S...>>
		{
			using Type = TTypeList<T..., S...>;
		};
		template<typename... S>
		struct TPrepend
		{
			using Type = TTypeList<S..., T...>;
		};
		template<typename... S>
		struct TPrepend<TTypeList<S...>>
		{
			using Type = TTypeList<S..., T...>;
		};

		template<typename... S>
		struct TAppendUnique : public Detail::TTypeListUnique<TTypeList<T...>, S...>
		{};
		template<typename... S>
		struct TAppendUnique<TTypeList<S...>>
		    : public Detail::TTypeListUnique<TTypeList<T...>, S...>
		{};

		template<typename... S>
		struct TPrependUnique : public Detail::TTypeListUnique<TTypeList<S...>, T...>
		{};
		template<typename... S>
		struct TPrependUnique<TTypeList<S...>>
		    : public Detail::TTypeListUnique<TTypeList<S...>, T...>
		{};

		template<typename... Args>
		struct TFirst : TypeIdentity<void>
		{};
		template<typename First, typename... Rest>
		struct TFirst<First, Rest...> : TypeIdentity<First>
		{};


	public:
		template<template<typename> class W>
		using Wrap = TTypeList<W<T>...>;

		template<template<typename> class W>
		using WrapPtr = TTypeList<W<T>*...>;

		template<template<typename> class W>
		using WrapConstPtr = TTypeList<const W<T>*...>;

		template<template<typename> class W>
		using WrapRef = TTypeList<W<T>&...>;

		template<template<typename> class W>
		using WrapConstRef = TTypeList<const W<T>&...>;

		template<template<typename> class M>
		using Map = TTypeList<typename M<T>::Type...>;

		template<typename... S>
		using Append = TAppend<S...>::Type;

		template<typename... S>
		using Prepend = TPrepend<S...>::Type;

		template<typename... S>
		using AppendUnique = TAppendUnique<S...>::Type;

		template<typename... S>
		using PrependUnique = TPrependUnique<S...>::Type;

		using Deduplicate = typename Detail::template TTypeListUnique<TTypeList<>, T...>::Type;

		// Conversion
		template<template<typename...> class U>
		using To = U<T...>;

		template<typename Default = void>
		using First = typename TFirst<Default>::Type;

		template<template<typename> typename Predicate>
		using Filter = Detail::TFilter<TTypeList<T...>, Predicate>::Type;

		static constexpr auto Call(auto predicate)
		{
			return predicate.template operator()<T...>();
		}
		static constexpr void ForEach(auto predicate)
		{
			(predicate.template operator()<T>(), ...);
		}
	};

	template<typename... T, typename... Other>
	constexpr TTypeList<T..., Other...> operator+(TTypeList<T...>, TTypeList<Other...>)
	{
		return {};
	}


	template<std::size_t, typename>
	struct TTypeListIterator;

	template<std::size_t index, typename T, typename... Other>
	struct TTypeListIterator<index, TTypeList<T, Other...>>
	    : TTypeListIterator<index - 1u, TTypeList<Other...>>
	{};

	template<typename T, typename... Other>
	struct TTypeListIterator<0u, TTypeList<T, Other...>>
	{
		/*! @brief Searched type. */
		using Type = T;
	};


	/**
	 * @brief Helper type.
	 * @tparam Index Index of the type to return.
	 * @tparam List Type list to search into.
	 */
	template<std::size_t index, typename List>
	using TTypeListIndex = typename TTypeListIterator<index, List>::Type;
}    // namespace p
