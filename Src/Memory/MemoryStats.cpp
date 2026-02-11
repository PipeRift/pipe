// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/String.h"
#include "Pipe/Core/Utility.h"
#include "PipeMath.h"

#include <algorithm>
#include <mutex>


namespace p
{
	// Use a custom arena that doesn't track allocations. Otherwise tracking stats would loop
	class P_API MemoryStatsArena : public Arena
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

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<MemoryStatsArena>();
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

	void MemoryStats::PrintAllocationError(StringView error, AllocationStats* allocation)
	{
		String msg;
		Strings::FormatTo(msg, error);

		if (allocation)
		{
			Strings::FormatTo(msg, " ({} {})", static_cast<void*>(allocation->ptr),
			    Strings::ParseMemorySize(allocation->size));
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
				PrintAllocationError("", &allocations[i]);
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
