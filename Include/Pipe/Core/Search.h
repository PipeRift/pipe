// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Greater.h"
#include "Pipe/Core/Heap.h"
#include "Pipe/Core/Less.h"
#include "Pipe/Core/Optional.h"
#include "Pipe/Core/Profiler.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"


namespace p::core
{
	/**
	 * Finds the position of the first element in a sorted range that has a value
	 * greater than or equivalent to a specified value.
	 *
	 * @param sortPredicate used to sort the array. Default: a < b
	 * @return the furthermost iterator i in the range [first, last) such that
	 * for any iterator j in the range [first, i) the following corresponding
	 * condition holds: sortPredicate(*j, value) == false.
	 *
	 * Complexity: worst O(log(n)) | best O(1)
	 */
	template<typename T, typename Index, typename Value, typename SortPredicate = TLess<>>
	Index LowerBound(
	    T* data, Index first, Index size, const Value& value, SortPredicate sortPredicate = {})
	{
		const Index last = first + size;

		while (size > 0)
		{
			// We use '>>1' here instead of '/2' because MSVC++ for some reason
			// generates significantly worse code for '/2'.
			const Index halfSize = size >> 1;

			const Index mid = first + halfSize;
			if (sortPredicate(data[mid], value))
			{
				first = mid + 1;
				size -= halfSize + 1;
			}
			else
			{
				size = halfSize;
			}
		}
		return first != last ? first : NO_INDEX;
	}

	/**
	 * Finds the position of the first element in a sorted range that has a
	 * value that is greater than a specified value.
	 *
	 * @param sortPredicate used to sort the array. Default: a < b
	 * @return the furthermost iterator i in the range [first, last) such that
	 * for any iterator j in the range [first, i) the following corresponding
	 * condition holds: sortPredicate(value, *j) == false.
	 *
	 * Complexity: worst O(log(n)) | best O(1)
	 */
	template<typename T, typename Index, typename Value, typename SortPredicate = TLess<>>
	Index UpperBound(
	    T* data, Index first, Index size, const Value& value, SortPredicate sortPredicate = {})
	{
		const Index last = first + size;

		while (size > 0)
		{
			// We use '>>1' here instead of '/2' because MSVC++ for some reason generates
			// significantly worse code for '/2'.
			const Index halfSize = size >> 1;

			const Index mid = first + halfSize;
			if (!sortPredicate(value, data[mid]))
			{
				first = mid + 1;
				size -= halfSize + 1;
			}
			else
			{
				size = halfSize;
			}
		}
		return first != last ? first : NO_INDEX;
	}

	/**
	 * Finds the position of the first element in a sorted range that has a
	 * value that is equal to a specified value.
	 *
	 * @param sortPredicate used to sort the array. Default: a < b
	 * @return true if there is an iterator i in the range [first, last) that
	 * satisfies the corresponding conditions: sortPredicate(*i, value) == false &&
	 * sortPredicate(value, *i) == false.
	 *
	 * Complexity: worst O(log(n)) | best O(1)
	 */
	template<typename T, typename Index, typename Value, typename SortPredicate = TLess<>>
	Index BinarySearch(
	    T* data, Index first, Index size, const Value& value, SortPredicate sortPredicate = {})
	{
		const Index it = LowerBound(data, first, size, value, sortPredicate);
		if (it != NO_INDEX && !sortPredicate(value, data[it]))
		{
			return it;
		}
		return NO_INDEX;
	}


	template<typename T, typename Index, typename Value>
	Index FindSortedMax(T* data, Index first, Index size, const Value& max, bool included = false)
	{
		if (first == NO_INDEX || size == 0)
		{
			return NO_INDEX;
		}

		const Index last = first + size - 1;
		const T& firstV  = data[first];
		const T& lastV   = data[last];
		if (firstV < lastV)    // First is smaller than last. Order is small to big
		{
			// Check Limits
			if (firstV >= max)
			{
				return (included && firstV == max) ? first : NO_INDEX;
			}
			else if (lastV <= max)
			{
				const bool moveDown = !included && lastV == max;
				return last - Index(moveDown);
			}

			auto i     = LowerBound(data, first, size, max, TLess<>());
			const T& v = data[i];
			if (v < max || (v == max && included))
			{
				return i;
			}
			return i - 1;
		}
		else if (firstV > lastV)
		{
			// Check Limits
			if (firstV <= max)
			{
				const bool moveUp = !included && firstV == max;
				return first + Index(moveUp);
			}
			else if (lastV >= max)
			{
				return (included && lastV == max) ? last : NO_INDEX;
			}

			auto i = UpperBound(data, first, size, max, TGreater<>());
			--i;
			const T& v = data[i];
			if (v < max || (v == max && included))
			{
				return i;
			}
			return i + 1;
		}

		// If first and last values are equal, we can just compare one value
		return firstV < max || (included && firstV == max) ? first : NO_INDEX;
	}


	template<typename T, typename Index, typename Value>
	Index FindSortedMin(T* data, Index first, Index size, const Value& min, bool included = false)
	{
		if (first == NO_INDEX || size == 0)
		{
			return NO_INDEX;
		}

		const Index last = first + size - 1;
		const T& firstV  = data[first];
		const T& lastV   = data[last];
		if (firstV < lastV)    // First is smaller than last. Order is small to big
		{
			// Check Limits
			if (firstV >= min)
			{
				const bool moveUp = !included && firstV == min;
				return first + Index(moveUp);
			}
			else if (lastV <= min)
			{
				return (included && lastV == min) ? last : NO_INDEX;
			}

			auto i = UpperBound(data, first, size, min, TLess<>());
			--i;
			const T& v = data[i];
			if (v > min || (v == min && included))
			{
				return i;
			}
			return i + 1;
		}
		else if (firstV > lastV)
		{
			// Check Limits
			if (firstV <= min)
			{
				return (included && firstV == min) ? first : NO_INDEX;
			}
			else if (lastV >= min)
			{
				const bool moveDown = !included && lastV == min;
				return last - Index(moveDown);
			}

			auto i     = LowerBound(data, first, size, min, TGreater<>());
			const T& v = data[i];
			if (v > min || (v == min && included))
			{
				return i;
			}
			return i - 1;
		}

		// If first and last values are equal, we can just compare one value
		return firstV > min || (included && firstV == min) ? first : NO_INDEX;
	}
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
