// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Backward.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Set.h"
#include "Pipe/Memory/IAllocator.h"

#include <cstdlib>
#include <shared_mutex>


namespace p
{
	// Use a custom arena that doesn't track allocations. Otherwise tracking stats would loop
	class PIPE_API MemoryStatsArena : public Arena
	{
	public:
		MemoryStatsArena()
		{
			Interface<MemoryStatsArena, &MemoryStatsArena::Alloc, &MemoryStatsArena::Alloc,
			    &MemoryStatsArena::Resize, &MemoryStatsArena::Free>();
		}
		~MemoryStatsArena() override = default;

		inline void* Alloc(const sizet size)
		{
			return std::malloc(size);
		}
		inline void* Alloc(const sizet size, const sizet align)
		{
			return std::malloc(size);
		}
		inline bool Resize(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		inline void Free(void* ptr, sizet size)
		{
			std::free(ptr);
		}
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
		mutable std::shared_mutex mutex;
		std::vector<AllocationStats> allocations;
#if P_ENABLE_ALLOCATION_STACKS
		std::vector<backward::StackTrace<std::allocator>> allocationStacks{};
#endif

		~MemoryStats();

		void Add(void* ptr, sizet size);
		void Remove(void* ptr);
	};
}    // namespace p
