// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <cassert>
#include <utility>


namespace Rift
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
}    // namespace Rift