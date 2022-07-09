// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/BigBestFitArena.h"

#include "Pipe/Core/Greater.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Search.h"
#include "Pipe/Core/Sorting.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	bool operator==(const BigBestFitArena::Slot& a, sizet b)
	{
		return a.Size() == b;
	}
	bool operator<(const BigBestFitArena::Slot& a, sizet b)
	{
		return a.Size() < b;
	}
	bool operator>(const BigBestFitArena::Slot& a, sizet b)
	{
		return a.Size() > b;
	}
	bool operator<=(const BigBestFitArena::Slot& a, sizet b)
	{
		return a.Size() <= b;
	}
	bool operator>=(const BigBestFitArena::Slot& a, sizet b)
	{
		return a.Size() >= b;
	}
	bool operator==(sizet a, const BigBestFitArena::Slot& b)
	{
		return a == b.Size();
	}
	bool operator<(sizet a, const BigBestFitArena::Slot& b)
	{
		return a < b.Size();
	}
	bool operator>(sizet a, const BigBestFitArena::Slot& b)
	{
		return a > b.Size();
	}
	bool operator<=(sizet a, const BigBestFitArena::Slot& b)
	{
		return a <= b.Size();
	}
	bool operator>=(sizet a, const BigBestFitArena::Slot& b)
	{
		return a >= b.Size();
	}

	BigBestFitArena::BigBestFitArena(Arena* parent, const sizet initialSize) : ChildArena(parent)
	{
		assert(initialSize > 0);
		block.data = p::Alloc(GetParentArena(), initialSize);
		block.size = initialSize;
		// Set address at end of block. Size is 0
		// freeSlots.SetData(static_cast<u8*>(block.GetData()) + block.Size());
		// Add first slot for the entire block
		freeSlots.Add(
		    {reinterpret_cast<u8*>(block.data), reinterpret_cast<u8*>(block.data) + block.size});

		freeSize = initialSize;
	}

	BigBestFitArena::~BigBestFitArena()
	{
		p::Free(block.data, block.size);
		block.data = nullptr;
	}

	void* BigBestFitArena::Alloc(const sizet size)
	{
		return Alloc(size, minAlignment);    // Always align by header size
	}

	void* BigBestFitArena::Alloc(const sizet size, sizet alignment)
	{
		// We always use at least 8 bytes of alignment for the header
		alignment = math::Max(alignment, minAlignment);

		const i32 slotIndex = FindSmallestSlot(size + alignment - 1);
		if (slotIndex == NO_INDEX || slotIndex >= freeSlots.Size())
		{
			// Log::Error("No slots can fit {} bytes!", size);
			return nullptr;
		}

		Slot& slot = freeSlots[slotIndex];

		u8* const ptr =
		    slot.start
		    + GetAlignmentPaddingWithHeader(slot.start, alignment, sizeof(AllocationHeader));

		auto* const header = GetHeader(ptr);
		header->end        = ptr + size;
		header->end += GetAlignmentPadding(header->end, minAlignment);    // Align end by 8

		ReduceSlot(slotIndex, slot, reinterpret_cast<u8*>(header), header->end);
		freeSize -= reinterpret_cast<sizet>(header->end) - reinterpret_cast<sizet>(header);
		return ptr;
	}

	void BigBestFitArena::Free(void* ptr, sizet size)
	{
		if (ptr)
		{
			auto* const header        = GetHeader(ptr);
			u8* const allocationStart = reinterpret_cast<u8*>(header);
			u8* const allocationEnd   = header->end;

			freeSize += allocationEnd - allocationStart;
			AbsorbFreeSpace(allocationStart, allocationEnd);
		}
	}

	i32 BigBestFitArena::FindSmallestSlot(sizet neededSize)
	{
		if (pendingSort) [[unlikely]]
		{
			pendingSort = false;
			if (float(freeSlots.Size()) / freeSlots.MaxSize() < 0.25f)
			{
				// Dont shrink until there is 75% of unused space
				freeSlots.Shrink();
			}

			// Sort slots by size. Small first
			freeSlots.Sort(TGreater<>());
		}

		// Find smallest slot fitting our required size
		return freeSlots.FindSortedMin(neededSize, true);
	}    // namespace p

	void BigBestFitArena::ReduceSlot(
	    i32 slotIndex, Slot& slot, u8* const allocationStart, u8* const allocationEnd)
	{
		if (allocationEnd == slot.end)    // Slot would become empty
		{
			if (allocationStart > slot.start)    // Slot can still fill alignment gap
			{
				slot.end    = allocationStart;
				pendingSort = true;
			}
			else
			{
				freeSlots.RemoveAtUnsafe(slotIndex, false);
			}
			return;
		}

		u8* const slotStart = slot.start;
		slot.start          = allocationEnd;
		if (allocationStart > slotStart)
		{
			// We are leaving a gap due to alignment, so add a new slot
			freeSlots.Add({slotStart, allocationStart});
		}
		pendingSort = true;
	}

	void BigBestFitArena::AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd)
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

			freeSlots.RemoveAtSwapUnsafe(previousSlot);
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
}    // namespace p
