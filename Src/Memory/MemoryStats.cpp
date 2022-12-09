// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/Heap.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"

#include <algorithm>


namespace p
{
	MemoryStats::~MemoryStats()
	{
		if (allocations.size() > 0)
		{
			TString<TChar> errorMsg;
			Strings::FormatTo(
			    errorMsg, "MEMORY LEAKS! {} allocations were not freed!", allocations.size());

			const sizet shown = math::Min<sizet>(64, allocations.size());
			for (i32 i = 0; i < shown; ++i)
			{
				const auto& allocation = allocations[i];
				Strings::FormatTo(errorMsg, "\n>{} {}", allocation.ptr,
				    Strings::ParseMemorySize(allocation.size));
#if P_ENABLE_ALLOCATION_STACKS
				const auto& stack = allocationStacks[i];
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

			if (shown < allocations.size())
			{
				Strings::FormatTo(
				    errorMsg, "\n...\n{} more not shown.", allocations.size() - shown);
			}
			std::puts(errorMsg.data());
		}
	}

	void MemoryStats::Add(void* ptr, sizet size)
	{
		std::unique_lock lock{mutex};
		used += size;
		const AllocationStats item{ptr, size};
		const auto it = allocations.insert(std::upper_bound(allocations.begin(), allocations.end(),
		                                       item, SortLessAllocationStats{}),
		    item);
		const i32 index = i32(it - allocations.begin());

#if P_ENABLE_ALLOCATION_STACKS
		allocationStacks.insert(allocationStacks.begin() + index, {});
		auto& stack = allocationStacks[index];
		stack.load_here(14 + 3);
		stack.skip_n_firsts(3);
#endif
		//  TracyAllocS(ptr, size, 8);
	}

	void MemoryStats::Remove(void* ptr)
	{
		std::unique_lock lock{mutex};
		// TracyFreeS(ptr, 8);
		const i32 index = std::binary_search(
		    allocations.begin(), allocations.end(), ptr, SortLessAllocationStats{});
		if (index != NO_INDEX)
		{
			used -= allocations[index].size;
			allocations.erase(std::next(allocations.begin(), index));
#if P_ENABLE_ALLOCATION_STACKS
			allocationStacks.erase(std::next(allocationStacks.begin(), index));
#endif
		}
	}
}    // namespace p
