// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Set.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Memory/STLAllocator.h"
#include "Pipe/PipeArrays.h"

// #if P_ENABLE_ALLOCATION_STACKS
#include "Pipe/Core/Backward.h"
// #endif

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
			Interface<MemoryStatsArena>();
		}

		inline void* Alloc(const sizet size)
		{
			return std::malloc(size);
		}
		inline void* Alloc(const sizet size, const sizet align)
		{
			return std::malloc(size);
		}
		inline bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
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
		u8* ptr  = nullptr;
		u64 size = 0;
	};

	struct PIPE_API SortLessAllocationStats
	{
		bool operator()(const AllocationStats& a, const AllocationStats& b) const
		{
			return a.ptr + a.size < b.ptr;
		}

		bool operator()(void* a, const AllocationStats& b) const
		{
			return a < b.ptr;
		}

		bool operator()(const AllocationStats& a, void* b) const
		{
			return a.ptr + a.size < b;
		}
	};


	struct PIPE_API MemoryStats
	{
		MemoryStatsArena arena;
		sizet used = 0;
		mutable std::shared_mutex mutex;
		std::vector<AllocationStats, STLAllocator<AllocationStats>> allocations;
#if P_ENABLE_ALLOCATION_STACKS
		// std::vector<backward::StackTrace, STLAllocator<backward::StackTrace>> stacks{};
#endif
		std::vector<AllocationStats, STLAllocator<AllocationStats>> freedAllocations;

		MemoryStats();
		~MemoryStats();

		void Add(void* ptr, sizet size);
		void Remove(void* ptr);

	private:
		void PrintAllocationError(
		    StringView error, AllocationStats* allocation, const backward::StackTrace* stack);
	};
}    // namespace p
