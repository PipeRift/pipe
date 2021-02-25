// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Arenas/BestFitArena.h"

#include "Log.h"
#include "Math/Math.h"
#include "Math/Search.h"
#include "Math/Sorting.h"
#include "Memory/Alloc.h"


namespace Rift::Memory
{
	void BestFitArena::SlotArray::Add(Slot&& slot)
	{
		--data;    // Point data at 1 item less.
		++size;
		data[0] = slot;
	}

	void BestFitArena::SlotArray::RemoveSwap(u32 index)
	{
		Swap(data[index], data[0]);
		++data;
		--size;
	}

	void BestFitArena::SlotArray::SortBySize()
	{
		Algorithms::Sort(data, size, [](const auto& a, const auto& b) {
			return a.end - a.start > b.end - b.start;
		});
	}

	u32 BestFitArena::SlotArray::FindSmallest(sizet minSlotSize)
	{
		return Algorithms::UpperBoundSearch(
		    data, size, minSlotSize, [](sizet minSlotSize, const auto& slot) {
			    return minSlotSize > slot.end - slot.start;
		    });
	}


	BestFitArena::BestFitArena(const sizet initialSize)
	{
		assert(initialSize > 0);
		block.Allocate(initialSize);
		// Set address at end of block. Size is 0
		freeSlots.SetData(static_cast<u8*>(block.GetData()) + block.GetSize());
		// Add first slot for the entire block
		freeSlots.Add({reinterpret_cast<u8*>(block.GetData()),
		    reinterpret_cast<u8*>(block.GetData()) + block.GetSize()});
	}

	void* BestFitArena::Allocate(const sizet size)
	{
		return Allocate(size, minAlignment);    // Always align by header size
	}

	void* BestFitArena::Allocate(const sizet size, sizet alignment)
	{
		// We always use at least 8 bytes of alignment
		alignment = Math::Max(alignment, minAlignment);

		if (pendingSort)
		{
			freeSlots.SortBySize();
			pendingSort = false;
		}

		u32 slotIndex = freeSlots.FindSmallest(size);
		assert(slotIndex < freeSlots.size && "No free slots!");

		Slot& slot = freeSlots.GetRef(slotIndex);

		const sizet padding =
		    GetAlignmentPaddingWithHeader(slot.start, alignment, sizeof(AllocationHeader));

		u8* ptr = slot.start + padding;

		auto* const header = GetHeader(ptr);

		header->end = ptr + size;
		header->end += GetAlignmentPadding(header->end, minAlignment);    // Align end by 8

		if (header->end >= reinterpret_cast<u8*>(freeSlots.data))
		{
			// Log::Error("Allocation doesn't fit!");
			return nullptr;
		}

		slot.start = header->end;
		// If empty, remove slot
		if (slot.start == slot.end)
		{
			freeSlots.RemoveSwap(slotIndex);
		}
		return ptr;
	}

	void BestFitArena::Free(void* ptr)
	{
		auto* const header = GetHeader(ptr);

		u8* const allocationStart = reinterpret_cast<u8*>(header);
		u8* const allocationEnd   = header->end;

		// Find previous and/or next slots
		bool hasPrevious = false;
		bool hasNext     = false;
		u32 previousSlot;
		u32 nextSlot;
		for (u32 i = 0; i < freeSlots.size; ++i)
		{
			const Slot& slot = freeSlots.GetRef(i);
			if (slot.start == allocationEnd)
			{
				hasNext  = true;
				nextSlot = i;
			}
			else if (slot.end == allocationStart)
			{
				hasPrevious  = true;
				previousSlot = i;
			}
		}

		if (hasPrevious && hasNext)
		{
			// Expand next slot to the start of the previous slot
			Slot& slot = freeSlots.GetRef(nextSlot);
			slot.start = freeSlots.GetRef(previousSlot).start;

			freeSlots.RemoveSwap(previousSlot);
		}
		else if (hasPrevious)
		{
			freeSlots.GetRef(previousSlot).start = allocationStart;
		}
		else if (hasNext)
		{
			freeSlots.GetRef(nextSlot).end = allocationEnd;
		}
		else
		{
			freeSlots.Add({allocationStart, allocationEnd});
		}
		pendingSort = true;
	}
}    // namespace Rift::Memory
