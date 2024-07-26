// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Utility.h"
#include "PipeMath.h"


namespace p
{
#pragma region Comparisons

	template<typename T = void>
	struct TGreater
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A > B;
		}
	};

	template<>
	struct TGreater<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) > Forward<U>(B);
		}
	};

	template<typename T = void>
	struct TGreaterEqual
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A >= B;
		}
	};

	template<>
	struct TGreaterEqual<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) >= Forward<U>(B);
		}
	};

	template<typename T = void>
	struct TLess
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A < B;
		}
	};

	template<>
	struct TLess<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) < Forward<U>(B);
		}
	};

	template<typename T = void>
	struct TLessEqual
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return A <= B;
		}
	};

	template<>
	struct TLessEqual<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return Forward<T>(A) <= Forward<U>(B);
		}
	};

	template<typename T = void>
	struct THashLess
	{
		constexpr bool operator()(const T& A, const T& B) const
		{
			return GetHash(A) < GetHash(B);
		}
	};

	template<>
	struct THashLess<void>
	{
		template<typename T, typename U>
		constexpr bool operator()(T&& A, U&& B) const
		{
			return GetHash(A) < GetHash(B);
		}
	};

#pragma endregion Comparisons


#pragma region Search
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
#pragma endregion Search


#pragma region Heap
	template<typename Index>
	Index HeapGetLeftChildIndex(Index index)
	{
		return index * 2 + 1;
	}

	/**
	 * Checks if node located at Index is a leaf or not.
	 *
	 * @param	index Node index.
	 * @returns	true if node is a leaf, false otherwise.
	 */
	template<typename Index>
	bool HeapIsLeaf(Index index, Index size)
	{
		return HeapGetLeftChildIndex(index) >= size;
	}

	/** @return the parent index of a node at Index. */
	template<typename Index>
	Index HeapGetParentIndex(Index index)
	{
		return (index - 1) / 2;
	}

	template<typename T, typename Index, typename Predicate>
	void HeapSiftDown(T* heap, Index index, const Index count, const Predicate& predicate)
	{
		while (!HeapIsLeaf(index, count))
		{
			const Index leftChildIndex  = HeapGetLeftChildIndex(index);
			const Index rightChildIndex = leftChildIndex + 1;

			Index minChildIndex = leftChildIndex;
			if (rightChildIndex < index)
			{
				minChildIndex = predicate(heap[leftChildIndex], heap[rightChildIndex])
				                  ? leftChildIndex
				                  : rightChildIndex;
			}
			if (!predicate(heap[minChildIndex], heap[index]))
			{
				break;
			}

			Swap(heap[index], heap[minChildIndex]);
			index = minChildIndex;
		}
	}

	template<typename T, typename Index, typename Predicate>
	Index HeapSiftUp(T* heap, Index rootIndex, Index nodeIndex, const Predicate& predicate)
	{
		while (nodeIndex > rootIndex)
		{
			i32 parentIndex = HeapGetParentIndex(nodeIndex);
			if (!predicate(heap[nodeIndex], heap[parentIndex]))
			{
				break;
			}

			Swap(heap[nodeIndex], heap[parentIndex]);
			nodeIndex = parentIndex;
		}
		return nodeIndex;
	}

	template<typename T, typename Index, typename Predicate>
	void Heapify(T* first, Index size, Predicate predicate)
	{
		for (Index i = HeapGetParentIndex(size - 1); i >= 0; --i)
		{
			HeapSiftDown(first, i, size, predicate);
		}
	}

	template<typename T, typename Index, typename Predicate>
	void HeapSort(T* first, Index size, Predicate predicate)
	{
		// Reverse the predicate to build a max-heap instead of a min-heap
		ReversePredicate<Predicate> reversePredicate(predicate);

		Heapify(first, size, reversePredicate);

		for (Index i = size - 1; i > 0; --i)
		{
			Swap(first[0], first[i]);

			HeapSiftDown(first, Index(0), i, reversePredicate);
		}
	}
#pragma endregion Heap


#pragma region Sort
	template<typename T, typename Index, typename Predicate>
	void Sort(T* first, Index size, Predicate predicate)
	{
		struct Stack
		{
			T* min;
			T* max;
			u32 maxDepth;
		};

		if (size < 2)
		{
			return;
		}

		Stack recursionStack[32] = {
		    {first, first + size - 1, u32(Log(float(size)) * 2.f)}
        };
		Stack current, inner;

		for (Stack* stackTop = recursionStack; stackTop >= recursionStack; --stackTop)
		{
			current = *stackTop;

		Loop:
			const Index count = Index(current.max - current.min + 1);

			if (current.maxDepth == 0)
			{
				// We're too deep into quick sort
				HeapSort(current.min, count, predicate);
				continue;
			}

			if (count <= 8)
			{
				// Use simple bubble-sort.
				while (current.max > current.min)
				{
					T *max, *item;
					for (max = current.min, item = current.min + 1; item <= current.max; item++)
					{
						if (predicate(*max, *item))
						{
							max = item;
						}
					}
					Swap(*max, *current.max--);
				}
			}
			else
			{
				// Grab middle element so sort doesn't exhibit worst-cast behavior with presorted
				// lists.
				Swap(current.min[count / 2], current.min[0]);

				// Divide list into two halves, one with items <=current.min, the other with items
				// >current.max.
				inner.min = current.min;
				inner.max = current.max + 1;
				for (;;)
				{
					while (++inner.min <= current.max && !predicate(*current.min, *inner.min))
						;
					while (--inner.max > current.min && !predicate(*inner.max, *current.min))
						;
					if (inner.min > inner.max)
					{
						break;
					}
					Swap(*inner.min, *inner.max);
				}
				Swap(*current.min, *inner.max);

				--current.maxDepth;

				// Save big half and recurse with small half.
				if (inner.max - 1 - current.min >= current.max - inner.min)
				{
					if (current.min + 1 < inner.max)
					{
						stackTop->min      = current.min;
						stackTop->max      = inner.max - 1;
						stackTop->maxDepth = current.maxDepth;
						stackTop++;
					}
					if (current.max > inner.min)
					{
						current.min = inner.min;
						goto Loop;
					}
				}
				else
				{
					if (current.max > inner.min)
					{
						stackTop->min      = inner.min;
						stackTop->max      = current.max;
						stackTop->maxDepth = current.maxDepth;
						stackTop++;
					}
					if (current.min + 1 < inner.max)
					{
						current.max = inner.max - 1;
						goto Loop;
					}
				}
			}
		}
	}
#pragma endregion Sort


	/** Generates CRC hash of the memory area */
	PIPE_API u32 MemCrc32(const void* Data, i32 Length, u32 CRC = 0);
}    // namespace p
