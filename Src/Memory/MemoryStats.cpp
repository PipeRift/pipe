// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Memory/MemoryStats.h"

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
	    : allocations{arena}
#if P_ENABLE_ALLOCATION_STACKS
	    , stacks{arena}
#endif
	{}

	MemoryStats::~MemoryStats()
	{
		if (allocations.Size() > 0)
		{
			TString<TChar> errorMsg;
			Strings::FormatTo(
			    errorMsg, "MEMORY LEAKS! {} allocations were not freed!", allocations.Size());

			const auto shown = math::Min<sizet>(64, allocations.Size());
			for (i32 i = 0; i < shown; ++i)
			{
				const auto& allocation = allocations[i];
				Strings::FormatTo(errorMsg, "\n>{} {}", allocation.ptr,
				    Strings::ParseMemorySize(allocation.size));
#if P_ENABLE_ALLOCATION_STACKS
				const auto& stack = stacks[i];
				backward::TraceResolver tr;
				tr.load_stacktrace(stack);
				for (sizet i = 0; i < stack.size(); ++i)
				{
					backward::ResolvedTrace trace = tr.resolve(stack[i]);
					Strings::FormatTo(errorMsg, "\n    #{} {} {} [{}]", i, trace.object_filename,
					    trace.object_function, trace.addr);
				}
#endif
			}

			if (shown < allocations.Size())
			{
				Strings::FormatTo(
				    errorMsg, "\n...\n{} more not shown.", allocations.Size() - shown);
			}
			std::puts(errorMsg.data());
		}
	}

	void MemoryStats::Add(void* ptr, sizet size)
	{
		// std::unique_lock lock{mutex};
		used += size;
		const AllocationStats item{static_cast<u8*>(ptr), size};
		i32 index = allocations.AddSorted(item, SortLessAllocationStats{});

#if P_ENABLE_ALLOCATION_STACKS
		stacks.Insert(index, {});
		auto& stack = stacks[index];
		stack.load_here(14 + 3);
		stack.skip_n_firsts(3);
#endif
	}

	void MemoryStats::Remove(void* ptr)
	{
		// std::unique_lock lock{mutex};
		const i32 index = allocations.FindSortedEqual(ptr, SortLessAllocationStats{});
		if (index != NO_INDEX)
		{
			used -= allocations[index].size;
			allocations.RemoveAt(index, false);
#if P_ENABLE_ALLOCATION_STACKS
			stacks.RemoveAt(index, false);
#endif
		}
		else
		{
			std::puts("Freeing a pointer that was not allocated or already freed!");
		}
	}
}    // namespace p
