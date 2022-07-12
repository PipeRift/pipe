// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"


namespace p
{
	MemoryStats::~MemoryStats()
	{
		if (allocations.Size() > 0)
		{
			TString<TChar, MemoryStatsAllocator> errorMsg;
			Strings::FormatTo(
			    errorMsg, "MEMORY LEAKS! {} allocations were not freed!", allocations.Size());

			const sizet shown = math::Min(64, allocations.Size());
			for (i32 i = 0; i < shown; ++i)
			{
				const auto& allocation = allocations[i];
				Strings::FormatTo(errorMsg, "\n>{} {}", allocation.ptr,
				    Strings::ParseMemorySize(allocation.size));
#if PIPE_ENABLE_ALLOCATION_STACKS
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
		std::unique_lock lock{mutex};
		used += size;
		const i32 index = allocations.AddSorted<SortLessAllocationStats>({ptr, size});

#if PIPE_ENABLE_ALLOCATION_STACKS
		allocationStacks.InsertDefaulted(index);
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
		const i32 index = allocations.FindSortedEqual<void*, SortLessAllocationStats>(ptr);
		if (index != NO_INDEX)
		{
			used -= allocations[index].size;
			allocations.RemoveAt(index, false);
#if PIPE_ENABLE_ALLOCATION_STACKS
			allocationStacks.RemoveAt(index, false);
#endif
		}
	}
}    // namespace p
