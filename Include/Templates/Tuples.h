// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <cassert>
#include <utility>


namespace Pipe
{
	template<typename T1, typename T2>
	using TPair = std::pair<T1, T2>;

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
}    // namespace Pipe
