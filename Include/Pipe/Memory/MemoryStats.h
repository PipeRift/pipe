// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Backward.h"
#include "Pipe/Core/Map.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Memory/IAllocator.h"

#include <cstdlib>


namespace p
{
	// Use a custom allocator that doesn't track allocations. Otherwise tracking stats would loop
	class MemoryStatsAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			T* Alloc(const sizet count)
			{
				return static_cast<T*>(std::malloc(sizeof(T) * count));
			}

			T* Alloc(const sizet count, const sizet align)
			{
				return static_cast<T*>(std::malloc(sizeof(T) * count));
			}

			bool Resize(T* ptr, sizet ptrCount, sizet count)
			{
				return false;
			}

			void Free(T* ptr, sizet count)
			{
				std::free(ptr);
			}
		};
	};


	struct PIPE_API AllocationStats
	{
		void* ptr  = nullptr;
		sizet size = 0;
		backward::StackTrace<MemoryStatsAllocator> stackTrace;

		AllocationStats(void* ptr, sizet size) : ptr{ptr}, size{size}
		{
			// Disabled until backward allocations are made safe
			stackTrace.load_here(10 + 3);
			stackTrace.skip_n_firsts(3);
		}
	};

	struct PIPE_API SortLessAllocationStats
	{
		bool operator()(const AllocationStats& a, const AllocationStats& b) const
		{
			return a.ptr < b.ptr;
		}

		bool operator()(void* a, const AllocationStats& b) const
		{
			return a < b.ptr;
		}

		bool operator()(const AllocationStats& a, void* b) const
		{
			return a.ptr < b;
		}
	};


	struct PIPE_API MemoryStats
	{
		sizet used = 0;

		TArray<AllocationStats, MemoryStatsAllocator> allocations;


		void Add(void* ptr, sizet size)
		{
			used += size;
			allocations.AddSorted<SortLessAllocationStats>({ptr, size});
			// TracyAllocS(ptr, size, 8);
		}

		void Remove(void* ptr)
		{
			// TracyFreeS(ptr, 8);
			const i32 index = allocations.FindSortedEqual<void*, SortLessAllocationStats>(ptr);
			if (index != NO_INDEX)
			{
				used -= allocations[index].size;
				allocations.RemoveAt(index, false);
			}
		}
	};
}    // namespace p
