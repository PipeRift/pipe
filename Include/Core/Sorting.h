// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Core/Heap.h"
#include "Core/Utility.h"
#include "Math/Math.h"
#include "Profiler.h"


namespace pipe::core
{
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
		    {first, first + size - 1, u32(math::Log(float(size)) * 2.f)}
        };
		Stack current, inner;

		for (Stack* stackTop = recursionStack; stackTop >= recursionStack; --stackTop)
		{
			current = *stackTop;

		Loop:
			Index count = Index(current.max - current.min + 1);

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
					T *max, *Item;
					for (max = current.min, Item = current.min + 1; Item <= current.max; Item++)
					{
						if (predicate(*max, *Item))
						{
							max = Item;
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
}    // namespace pipe::core

namespace pipe
{
	using namespace pipe::core;
}
