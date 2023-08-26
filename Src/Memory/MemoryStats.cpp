// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/Backward.h"
#include "Pipe/Core/Checks.h"
#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/Heap.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"

#include <algorithm>
#include <mutex>


namespace p
{
	MemoryStats::MemoryStats()
	    : allocations{STLAllocator<AllocationStats>{arena}}
#if P_ENABLE_ALLOCATION_STACKS
	    , stacks{STLAllocator<AllocationStats, STLAllocator<backward::StackTrace>>{arena}}
#endif
	    , freedAllocations{STLAllocator<AllocationStats>{arena}}
	{}

	MemoryStats::~MemoryStats()
	{
		if (allocations.size() > 0)
		{
			TString<TChar> errorMsg;
			Strings::FormatTo(
			    errorMsg, "MEMORY LEAKS! {} allocations were not freed!", allocations.size());

			const auto shown = math::Min<sizet>(64, allocations.size());
			for (i32 i = 0; i < shown; ++i)
			{
				PrintAllocationError("", &allocations[i], nullptr);
#if P_ENABLE_ALLOCATION_STACKS
				PrintAllocationError("", &allocations[i], &stacks[i]);
#endif
			}

			if (shown < allocations.size())
			{
				Strings::FormatTo(
				    errorMsg, "\n...\n{} more not shown.", allocations.size() - shown);
			}
			std::puts(errorMsg.data());
		}
	}

	void MemoryStats::PrintAllocationError(
	    StringView error, AllocationStats* allocation, const backward::StackTrace* stack)
	{
		String msg;
		Strings::FormatTo(msg, error);

		if (allocation)
		{
			Strings::FormatTo(msg, " ({} {})", (void*)allocation->ptr,
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
		// std::unique_lock lock{mutex};
		used += size;
		const AllocationStats item{static_cast<u8*>(ptr), size};
		allocations.insert(std::upper_bound(allocations.begin(), allocations.end(), item,
		                       SortLessAllocationStats{}),
		    item);

#if P_ENABLE_ALLOCATION_STACKS
		auto& stack = stacks.Insert(index, {arena});
		backward::StackTrace stack{arena};
		stack.load_here(14 + 3);
		stack.skip_n_firsts(3);
#endif
	}

	void MemoryStats::Remove(void* ptr)
	{
		if (!ptr)
		{
			return;
		}

		// std::unique_lock lock{mutex};
		const i32 index = p::BinarySearch(
		    allocations.data(), 0, i32(allocations.size()), ptr, SortLessAllocationStats{});
		if (index != NO_INDEX)
		{
			AllocationStats& allocation = allocations[index];
			used -= allocation.size;
			freedAllocations.insert(
			    std::upper_bound(freedAllocations.begin(), freedAllocations.end(), allocation,
			        SortLessAllocationStats{}),
			    Move(allocation));
			allocations.erase(allocations.begin() + index);
#if P_ENABLE_ALLOCATION_STACKS
			stacks.erase(stacks.begin() + index);
#endif
		}
		else
		{
			if (p::BinarySearch(freedAllocations.data(), 0, i32(freedAllocations.size()),
			        AllocationStats{static_cast<u8*>(ptr)}, SortLessAllocationStats{})
			    != NO_INDEX)
			{
				std::puts("Freeing a pointer more than once.");
			}
			else
			{
				std::puts("Freeing a pointer that was never allocated.");
			}
		}
	}
}    // namespace p
