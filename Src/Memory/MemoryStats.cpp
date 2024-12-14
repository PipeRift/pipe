// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/String.h"
#include "Pipe/Core/Utility.h"
#include "PipeMath.h"

#include <algorithm>
#include <mutex>


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

	// Getter to ensure readiness at static init time
	MemoryStatsArena& GetStateArena()
	{
		static MemoryStatsArena arena;
		return arena;
	}


	MemoryStats::MemoryStats() : allocations{GetStateArena()}, freedAllocations{GetStateArena()} {}

	MemoryStats::~MemoryStats()
	{
		Release();
	}

	void MemoryStats::PrintAllocationError(
	    StringView error, AllocationStats* allocation, const backward::StackTrace* stack)
	{
		String msg;
		Strings::FormatTo(msg, error);

		if (allocation)
		{
			Strings::FormatTo(msg, " ({} {})", static_cast<void*>(allocation->ptr),
			    Strings::ParseMemorySize(allocation->size));
		}

		if (stack)
		{
			backward::TraceResolver tr;
			tr.load_stacktrace(*stack);
			for (sizet i = 0; i < stack->size(); ++i)
			{
				backward::ResolvedTrace trace = tr.resolve((*stack)[i]);
				Strings::FormatTo(msg, "\n    #{} {} {} [{}]", i, trace.object_filename,
				    trace.object_function, trace.addr);
			}
		}
		std::puts(msg.data());
	}

	void MemoryStats::Add(void* ptr, sizet size)
	{
		std::unique_lock lock{mutex};
		used += size;
		const AllocationStats item{static_cast<u8*>(ptr), size};
		i32 index = allocations.AddSorted(item, SortLessAllocationStats{});
	}

	void MemoryStats::Remove(void* ptr, sizet size)
	{
		if (!ptr)
		{
			return;
		}

		used -= size;

		std::unique_lock lock{mutex};
		const i32 index = allocations.FindSorted(ptr, SortLessAllocationStats{});
		if (index != NO_INDEX)
		{
			AllocationStats& allocation = allocations[index];
			P_CheckMsg(size == allocation.size,
			    "Freed an allocation with a different size to which it got allocated with.");
			freedAllocations.AddSorted(Move(allocation), SortLessAllocationStats{});
			allocations.RemoveAt(index);
		}
		else
		{
			if (freedAllocations.ContainsSorted(ptr, SortLessAllocationStats{}))
			{
				std::puts("Freeing a pointer more than once.");
			}
			else
			{
				std::puts("Freeing a pointer that was never allocated.");
			}
		}
	}

	void MemoryStats::Release()
	{
		if (allocations.Size() > 0)
		{
			TString<char> errorMsg;
			Strings::FormatTo(
			    errorMsg, "{}: {} allocations were not freed!", name, allocations.Size());

			const auto shown = Min<sizet>(64, allocations.Size());
			for (i32 i = 0; i < shown; ++i)
			{
				PrintAllocationError("", &allocations[i], nullptr);
			}

			if (shown < allocations.Size())
			{
				Strings::FormatTo(
				    errorMsg, "\n...\n{} more not shown.", allocations.Size() - shown);
			}
			std::puts(errorMsg.data());
		}
		allocations.Clear();
		used = 0;
	}
}    // namespace p
