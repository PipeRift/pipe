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
	bool operator==(const BigBestFitArena::Slot& a, u32 b)
	{
		return a.size == b;
	}
	auto operator<=>(const BigBestFitArena::Slot& a, u32 b)
	{
		return a.size <=> b;
	}
	bool operator==(u32 a, const BigBestFitArena::Slot& b)
	{
		return a == b.size;
	}
	auto operator<=>(u32 a, const BigBestFitArena::Slot& b)
	{
		return a <=> b.size;
	}

	BigBestFitArena::BigBestFitArena(Arena* parent, const sizet initialSize) : ChildArena(parent)
	{
		Interface<BigBestFitArena, &BigBestFitArena::Alloc, &BigBestFitArena::Alloc,
		    &BigBestFitArena::Resize, &BigBestFitArena::Free>();

		assert(initialSize > 0);
		block.data = p::Alloc(GetParentArena(), initialSize);
		block.size = initialSize;
		// Set address at end of block. Size is 0
		// freeSlots.SetData(static_cast<u8*>(block.GetData()) + block.Size());
		// Add first slot for the entire block
		freeSlots.Add({0, block.size});

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

		u8* slotStart = (u8*)block.data + slot.offset;
		u8* const ptr =
		    slotStart
		    + GetAlignmentPaddingWithHeader(slotStart, alignment, sizeof(AllocationHeader));

		auto* const header = GetHeader(ptr);
		header->end        = ptr + size;
		header->end += GetAlignmentPadding(header->end, minAlignment);    // Align end by 8

		ReduceSlot(
		    slotIndex, slot, ToOffset(header, block.data), ToOffset(header->end, block.data));
		freeSize -= static_cast<u8*>(header->end) - reinterpret_cast<u8*>(header);
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
			AbsorbFreeSpace(
			    ToOffset(allocationStart, block.data), ToOffset(allocationEnd, block.data));
		}
	}

	i32 BigBestFitArena::FindSmallestSlot(sizet neededSize)
	{
		if (pendingSort) [[unlikely]]
		{
			pendingSort = false;
			if (float(freeSlots.Size()) / freeSlots.Capacity() < 0.25f)
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
	    i32 slotIndex, Slot& slot, u32 allocationStart, u32 allocationEnd)
	{
		if (allocationEnd == slot.offset + slot.size)    // Slot would become empty
		{
			if (allocationStart > slot.offset)    // Slot can still fill alignment gap
			{
				slot.size   = allocationStart - slot.offset;
				pendingSort = true;
			}
			else
			{
				freeSlots.RemoveAtUnsafe(slotIndex, false);
			}
			return;
		}

		u32 offset  = slot.offset;
		slot.offset = allocationEnd;
		slot.size += offset - allocationEnd;

		// If alignment leaves a gap in the slot, save this space as a new slot
		if (allocationStart > offset)
		{
			freeSlots.Add({offset, allocationStart - offset});
		}
		// If slot is smaller than prev slot, we have to sort
		else if (slotIndex > 0 && slot.size < freeSlots[slotIndex - 1].size)
		{
			pendingSort = true;
		}
	}

	void BigBestFitArena::AbsorbFreeSpace(u32 allocationStart, u32 allocationEnd)
	{
		// Find previous and/or next slots
		i32 previousSlot = NO_INDEX;
		i32 nextSlot     = NO_INDEX;
		for (i32 i = 0; i < freeSlots.Size(); ++i)
		{
			const Slot& slot = freeSlots[i];
			if (slot.offset == allocationEnd)
			{
				nextSlot = i;
				if (previousSlot != NO_INDEX)
				{
					break;    // We found both slots
				}
			}
			else if ((slot.offset + slot.size) == allocationStart)
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
			Slot& next     = freeSlots[nextSlot];
			Slot& previous = freeSlots[previousSlot];
			const Slot combined{previous.offset, (next.offset - previous.offset) + next.size};
			// Remove the smallest slot, expand the other
			if (next.size > previous.size)
			{
				next = combined;
				freeSlots.RemoveAtSwapUnsafe(previousSlot);
			}
			else
			{
				previous = combined;
				freeSlots.RemoveAtSwapUnsafe(nextSlot);
			}
		}
		else if (previousSlot != NO_INDEX)
		{
			Slot& previous = freeSlots[previousSlot];
			previous.size  = allocationEnd - previous.offset;
		}
		else if (nextSlot != NO_INDEX)
		{
			Slot& next = freeSlots[nextSlot];
			next.size += next.offset - allocationStart;
			next.offset = allocationStart;
		}
		else
		{
			freeSlots.Add({allocationStart, allocationEnd - allocationStart});
		}
		pendingSort = true;
	}

	u32 BigBestFitArena::ToOffset(void* data, void* block)
	{
		return u32(static_cast<u8*>(data) - static_cast<u8*>(block));
	}
}    // namespace p
