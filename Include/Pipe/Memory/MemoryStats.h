// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Backward.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Set.h"
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
#if PIPE_ENABLE_ALLOCATION_STACKS
		TArray<backward::StackTrace<MemoryStatsAllocator>, MemoryStatsAllocator> allocationStacks;
#endif


		~MemoryStats();

		void Add(void* ptr, sizet size);
		void Remove(void* ptr);
	};
}    // namespace p
