// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Arenas/BestFitArena.h"

#include "Log.h"
#include "Math/Math.h"
#include "Math/Search.h"
#include "Math/Sorting.h"
#include "Memory/Alloc.h"
#include "Misc/Utility.h"


namespace Rift::Memory
{
	BestFitArena::BestFitArena(const sizet initialSize)
	{
		assert(initialSize > 0);
		block.Allocate(initialSize);
		// Set address at end of block. Size is 0
		// freeSlots.SetData(static_cast<u8*>(block.GetData()) + block.GetSize());
		// Add first slot for the entire block
		freeSlots.Add({reinterpret_cast<u8*>(block.GetData()),
		    reinterpret_cast<u8*>(block.GetData()) + block.GetSize()});

		availableSpace = initialSize;
	}

	void* BestFitArena::Allocate(const sizet size)
	{
		return Allocate(size, minAlignment);    // Always align by header size
	}

	void* BestFitArena::Allocate(const sizet size, sizet alignment)
	{
		// We always use at least 8 bytes of alignment for the header
		alignment = Math::Max(alignment, minAlignment);

		const u32 slotIndex = FindSmallestSlot(size);
		if (slotIndex >= freeSlots.Size())
		{
			// Log::Error("No slots can fit {} bytes!", size);
			return nullptr;
		}

		Slot& slot = freeSlots[slotIndex];

		u8* const ptr = slot.start + GetAlignmentPaddingWithHeader(
		                                 slot.start, alignment, sizeof(AllocationHeader));

		auto* const header = GetHeader(ptr);
		header->end = ptr + size;
		header->end += GetAlignmentPadding(header->end, minAlignment);    // Align end by 8
		if (header->end > block.GetEnd())
		{
			// Log::Error("Allocation doesn't fit!");
			return nullptr;
		}

		ReduceSlot(slotIndex, slot, header->end);

		availableSpace -= reinterpret_cast<sizet>(slot.start) - reinterpret_cast<sizet>(header);
		return ptr;
	}

	void BestFitArena::Free(void* ptr)
	{
		auto* const header        = GetHeader(ptr);
		u8* const allocationStart = reinterpret_cast<u8*>(header);
		u8* const allocationEnd   = header->end;

		availableSpace += allocationEnd - allocationStart;
		AbsorbFreeSpace(allocationStart, allocationEnd);
	}

	i32 BestFitArena::FindSmallestSlot(sizet size)
	{
		if (pendingSort)
		{
			// Sort slots by size. Small first
			freeSlots.Sort([](const auto& a, const auto& b) {
				return a.end - a.start > b.end - b.start;
			});
			pendingSort = false;
		}

		// Find smallest slot fitting our required size
		return Algorithms::UpperBoundSearch(
		    freeSlots.Data(), freeSlots.Size(), size, [](sizet desiredSize, const auto& slot) {
			    return desiredSize < slot.end - slot.start;
		    });
	}

	void BestFitArena::ReduceSlot(i32 slotIndex, Slot& slot, u8* const allocationEnd)
	{
		slot.start = allocationEnd;
		if (slot.start == slot.end)
		{
			// If slot is empty, remove it
			freeSlots.RemoveAtSwap(slotIndex, false);
			// TODO: Investigate if Swap in this case is decremental since it requires sorting
			pendingSort = true;
		}
	}

	void BestFitArena::AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd)
	{
		// Find previous and/or next slots
		i32 previousSlot = NO_INDEX;
		i32 nextSlot     = NO_INDEX;
		for (i32 i = 0; i < freeSlots.Size(); ++i)
		{
			const Slot& slot = freeSlots[i];
			if (slot.start == allocationEnd)
			{
				nextSlot = i;
				if (previousSlot != NO_INDEX)
				{
					break;    // We found both slots
				}
			}
			else if (slot.end == allocationStart)
			{
				previousSlot = i;
				if (nextSlot != NO_INDEX)
				{
					break;    // We found both slots
				}
			}
		}

		if (previousSlot != NO_INDEX && nextSlot != NO_INDEX)
		{
			// Expand next slot to the start of the previous slot
			Slot& slot = freeSlots[nextSlot];
			slot.start = freeSlots[previousSlot].start;

			freeSlots.RemoveAtSwap(previousSlot);
		}
		else if (previousSlot != NO_INDEX)
		{
			freeSlots[previousSlot].end = allocationEnd;
		}
		else if (nextSlot != NO_INDEX)
		{
			freeSlots[nextSlot].start = allocationStart;
		}
		else
		{
			freeSlots.Add({allocationStart, allocationEnd});
		}
		pendingSort = true;
	}
}    // namespace Rift::Memory
