// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Math/Heap.h"
#include "Math/Math.h"
#include "Misc/Optional.h"
#include "Misc/Utility.h"


namespace Rift::Algorithms
{
	/**
	 * Performs binary search, resulting in position of the first element >= Value using predicate
	 *
	 * @param data containing the elements
	 * @param size of the data. Number of elements
	 * @param value Value to look for
	 * @param sortPredicate Predicate for sort comparison, defaults to <
	 *
	 * @returns Position of the first element >= Value, may be position after last element in range
	 */
	template <typename T, typename Index, typename ValueType, typename SortPredicate>
	Index LowerBoundSearch(T* data, Index size, const ValueType& value, SortPredicate sortPredicate)
	{
		Index start = 0;    // Current start of sequence to check
		// "size" will become the size of sequence to check

		// With this method, if size is even it will do one more comparison than necessary, but
		// because size can be predicted by the CPU it is faster in practice
		while (size > 0)
		{
			const Index leftoverSize = size % 2;
			size                     = size / 2;

			const Index checkIndex  = start + size;
			const Index startIfLess = checkIndex + leftoverSize;

			start = sortPredicate(data[checkIndex], value) ? startIfLess : start;
		}
		return start;
	}

	/**
	 * Performs binary search, resulting in position of the first element > Value using predicate
	 *
	 * @param data containing the elements
	 * @param size of the data. Number of elements
	 * @param value Value to look for
	 * @param sortPredicate Predicate for sort comparison, defaults to <
	 *
	 * @returns Position of the first element > Value, may be past end of range
	 */
	template <typename T, typename Index, typename ValueType, typename SortPredicate>
	Index UpperBoundSearch(T* data, Index size, const ValueType& value, SortPredicate sortPredicate)
	{
		Index start = 0;    // Current start of sequence to check
		// "size" will become the size of sequence to check

		// With this method, if Size is even it will do one more comparison than necessary, but
		// because size can be predicted by the CPU it is faster in practice
		while (size > 0)
		{
			const Index leftoverSize = size % 2;
			size                     = size / 2;

			const Index checkIndex  = start + size;
			const Index startIfLess = checkIndex + leftoverSize;

			start = !sortPredicate(value, data[checkIndex]) ? startIfLess : start;
		}

		return start;
	}

	/**
	 * Returns the index of the first found element matching a value.
	 * Elements must be sorted by sortPredicate
	 *
	 * @param data containing the elements
	 * @param size of the data. Number of elements
	 * @param value to search for
	 * @param sortPredicate Predicate for sort comparison, defaults to <
	 * @return index of found element. If not found, optional is not set
	 */
	template <typename T, typename Index, typename ValueType, typename SortPredicate>
	Optional<Index> BinarySearch(
	    T* data, Index size, const ValueType& value, SortPredicate sortPredicate)
	{
		const Index checkIndex = LowerBoundSearch(data, size, value, sortPredicate);
		if (checkIndex < size)
		{
			// Since we returned lower bound we already know Value <= CheckValue. So if Value is not
			// < checkValue, they must be equal
			if (!sortPredicate(value, data[checkIndex]))
			{
				return checkIndex;
			}
		}
		return {};
	}
}    // namespace Rift::Algorithms
