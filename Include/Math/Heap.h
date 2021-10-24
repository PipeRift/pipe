// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Misc/Utility.h"


namespace Rift::Algorithms
{
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
}    // namespace Rift::Algorithms
