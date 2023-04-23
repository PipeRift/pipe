// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/BestFitArena.h"

#include "Pipe/Core/Greater.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Profiler.h"
#include "Pipe/Core/Search.h"
#include "Pipe/Core/Sorting.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	bool operator==(const BestFitArena::Slot& a, sizet b)
	{
		return a.size == b;
	}
	bool operator<(const BestFitArena::Slot& a, sizet b)
	{
		return a.size < b;
	}
	bool operator>(const BestFitArena::Slot& a, sizet b)
	{
		return a.size > b;
	}
	bool operator<=(const BestFitArena::Slot& a, sizet b)
	{
		return a.size <= b;
	}
	bool operator>=(const BestFitArena::Slot& a, sizet b)
	{
		return a.size >= b;
	}
	bool operator==(sizet a, const BestFitArena::Slot& b)
	{
		return a == b.size;
	}
	bool operator<(sizet a, const BestFitArena::Slot& b)
	{
		return a < b.size;
	}
	bool operator>(sizet a, const BestFitArena::Slot& b)
	{
		return a > b.size;
	}
	bool operator<=(sizet a, const BestFitArena::Slot& b)
	{
		return a <= b.size;
	}
	bool operator>=(sizet a, const BestFitArena::Slot& b)
	{
		return a >= b.size;
	}


	BestFitArena::BestFitArena(Arena* parent, const sizet initialSize) : ChildArena(parent)
	{
		Interface<BestFitArena, &BestFitArena::Alloc, &BestFitArena::Alloc, &BestFitArena::Resize,
		    &BestFitArena::Free>();

		assert(initialSize > 0);
		block.data = p::Alloc(GetParentArena(), initialSize);
		block.size = initialSize;
		// Set address at end of block. Size is 0
		// freeSlots.SetData(static_cast<u8*>(block.GetData()) + block.Size());
		// Add first slot for the entire block
		freeSlots.Add({reinterpret_cast<u8*>(block.data), block.size});

		freeSize = initialSize;
	}

	BestFitArena::~BestFitArena()
	{
		p::Free(block.data, block.size);
		block.data = nullptr;
	}

	void* BestFitArena::Alloc(const sizet size)
	{
		const i32 slotIndex = FindSmallestSlot(size);
		if (slotIndex == NO_INDEX || slotIndex >= freeSlots.Size()) [[unlikely]]
		{
			// Error("Couldn't fit {} bytes", size);
			return nullptr;
		}

		Slot& slot      = freeSlots[slotIndex];
		u8* const start = slot.start;
		u8* const end   = start + size;

		ReduceSlot(slotIndex, slot, start, end);
		freeSize -= size;
		return start;
	}

	void* BestFitArena::Alloc(const sizet size, sizet alignment)
	{
		// Maximum size needed, based on worst possible alignment:
		const i32 slotIndex = FindSmallestSlot(size + (alignment - 1));
		if (slotIndex == NO_INDEX || slotIndex >= freeSlots.Size()) [[unlikely]]
		{
			// Error("Couldn't fit {} bytes", size);
			return nullptr;
		}

		Slot& slot      = freeSlots[slotIndex];
		u8* const start = slot.start + GetAlignmentPadding(slot.start, alignment);
		u8* const end   = start + size;

		ReduceSlot(slotIndex, slot, start, end);
		freeSize -= size;
		return start;
	}

	void BestFitArena::Free(void* ptr, sizet size)
	{
		if (ptr)
		{
			u8* const allocationStart = static_cast<u8*>(ptr);
			u8* const allocationEnd   = allocationStart + size;
			freeSize += allocationEnd - allocationStart;
			AbsorbFreeSpace(allocationStart, allocationEnd);
		}
	}

	i32 BestFitArena::FindSmallestSlot(sizet neededSize)
	{
		if (pendingSort) [[unlikely]]
		{
			pendingSort = false;
			if (float(freeSlots.Size()) / freeSlots.Capacity() < 0.1f)
			{
				// Dont shrink until there is 90% of unused space
				freeSlots.Shrink();
			}
			// Sort slots by size. Small first
			freeSlots.Sort(TGreater<>());
		}

		// Find smallest slot fitting our required size
		return freeSlots.FindSortedMin(neededSize, true);
		// return freeSlots.UpperBound(neededSize);
	}

	void BestFitArena::ReduceSlot(
	    i32 slotIndex, Slot& slot, u8* const allocationStart, u8* const allocationEnd)
	{
		if (allocationEnd == slot.End())    // Slot would become empty
		{
			if (allocationStart > slot.start)    // Slot can still fill alignment gap
			{
				slot.size   = allocationStart - slot.start;
				pendingSort = true;
			}
			else
			{
				freeSlots.RemoveAtUnsafe(slotIndex, false);
			}
			return;
		}

		u8* const oldSlotStart = slot.start;
		slot.start             = allocationEnd;
		slot.size += oldSlotStart - allocationEnd;

		// If alignment leaves a gap in the slot, save this space as a new slot
		if (allocationStart > oldSlotStart)
		{
			freeSlots.Add({oldSlotStart, sizet(allocationStart - oldSlotStart)});
			pendingSort = true;
		}
		// If slot is smaller than prev slot, we have to sort
		else if (slotIndex > 0 && slot.size < freeSlots[slotIndex - 1].size)
		{
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
			else if (slot.End() == allocationStart)
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
			const Slot& next = freeSlots[nextSlot];
			Slot& previous   = freeSlots[previousSlot];
			previous.size    = next.End() - previous.start;

			freeSlots.RemoveAtSwapUnsafe(nextSlot);
		}
		else if (previousSlot != NO_INDEX)
		{
			Slot& previous = freeSlots[previousSlot];
			previous.size  = allocationEnd - previous.start;
		}
		else if (nextSlot != NO_INDEX)
		{
			Slot& next = freeSlots[nextSlot];
			next.size += next.start - allocationStart;
			next.start = allocationStart;
		}
		else
		{
			freeSlots.Add({allocationStart, sizet(allocationEnd - allocationStart)});
		}
		pendingSort = true;
	}
}    // namespace p
