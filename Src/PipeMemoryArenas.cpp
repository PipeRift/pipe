// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeMemoryArenas.h"


namespace p
{
#pragma region Heap Arena
	HeapArena::HeapArena()
	{
		stats.name = "Heap Arena";
		Interface<HeapArena>();
	}

	void* HeapArena::Alloc(const sizet size)
	{
		void* ptr = p::HeapAlloc(size);
		stats.Add(ptr, size);
		return ptr;
	}
	void* HeapArena::Alloc(const sizet size, const sizet align)
	{
		void* ptr = p::HeapAlloc(size, align);
		stats.Add(ptr, size);
		return ptr;
	}
	bool HeapArena::Realloc(void* ptr, const sizet ptrSize, const sizet size)
	{
		return false;
	}
	void HeapArena::Free(void* ptr, sizet size)
	{
		stats.Remove(ptr, size);
		p::HeapFree(ptr);
	}
#pragma endregion Heap Arena

#pragma region Mono Linear
	MonoLinearArena::MonoLinearArena(ArenaBlock externalBlock, Arena& parentArena)
	    : ChildArena(&parentArena), insert{externalBlock.data}, block{Move(externalBlock)}
	{
		stats.name = "Mono Linear Arena";
		Interface<MonoLinearArena>();
	}

	MonoLinearArena::MonoLinearArena(const sizet blockSize, Arena& parentArena)
	    : ChildArena(&parentArena)
	    , insert{GetParentArena().Alloc(blockSize)}
	    , block{insert, blockSize}
	    , selfAllocated{true}
	{
		stats.name = "Mono Linear Arena";
		Interface<MonoLinearArena>();
	}

	void* MonoLinearArena::Alloc(sizet size)
	{
		// TODO: Resolve a reasonable align size based on allocation size
		return Alloc(size, alignof(std::max_align_t));
	}

	void* MonoLinearArena::Alloc(sizet size, sizet align)
	{
		u8* const allocEnd = (u8*)insert + size + GetAlignmentPadding(insert, align);
		void* ptr;
		// Not enough space in current block?
		if (allocEnd <= block.End()) [[likely]]
		{
			insert = allocEnd;
			++count;
			ptr = allocEnd - size;    // Fast-path
		}
		else
		{
			// Allocation doesn't fit. Allocate in parent arena
			ptr = GetParentArena().Alloc(size, align);
		}
		stats.Add(ptr, size);
		return ptr;
	}

	void MonoLinearArena::Free(void* ptr, sizet size)
	{
		stats.Remove(ptr, size);
		if (ptr >= block.data && ptr < block.End()) [[likely]]
		{
			--count;
			if (count <= 0) [[unlikely]]
			{
				Release(true);
			}
		}
		else
		{
			GetParentArena().Free(ptr, size);
		}
	}

	void MonoLinearArena::Release(bool keepIfSelfAllocated)
	{
		stats.Release();
		insert = block.data;
		count  = 0;
		if (selfAllocated && !keepIfSelfAllocated)
		{
			// Self allcoated block gets freed
			GetParentArena().Free(block.data, block.size);
			block = {};
		}
	}
#pragma endregion Mono Linear

#pragma region Multi Linear
	template<sizet blockSize>
	void Details::LinearBasePool<blockSize>::AllocateBlock(Arena& parentArena)
	{
		LinearBlock* const lastBlock = freeBlock;

		// Allocate aligning by blockSize
		void* ptr = parentArena.Alloc(blockSize + sizeof(LinearBlock), GetBlockSize());

		void* blockPtr       = (u8*)ptr + GetAlignmentPadding(ptr, GetBlockSize());
		freeBlock            = new (blockPtr) LinearBlock();
		insert               = freeBlock + 1;
		freeBlock->count     = 1;    // Count +1 prevents Free cleaning blocks being filled
		freeBlock->last      = lastBlock;
		freeBlock->unaligned = ptr;

		if (lastBlock)
		{
			lastBlock->next = freeBlock;
			// Dismark block allowing freeing it:
			--lastBlock->count;
			// In theory, we would need to free lastBlock if full here...
			// But we only reach AllocateBlock if the previous one is full!
		}
	}

	template<sizet blockSize>
	void Details::LinearBasePool<blockSize>::FreeBlock(Arena& parentArena, LinearBlock* block)
	{
		P_Check(block);
		// freeBlock is never freed. No need to handle it.
		if (block->next)
		{
			block->next->last = block->last;
		}
		if (block->last)
		{
			block->last->next = block->next;
		}
		parentArena.Free(block->unaligned, GetAllocatedBlockSize());
	}

	template<sizet blockSize>
	void* Details::LinearBasePool<blockSize>::Alloc(Arena& parentArena, sizet size, sizet align)
	{
		insert = (u8*)insert + size + GetAlignmentPadding(insert, align);

		// Not enough space in current block?
		if (freeBlock && insert <= GetBlockEnd(freeBlock)) [[likely]]
		{
			++freeBlock->count;
			return (u8*)insert - size;    // Fast-path
		}

		AllocateBlock(parentArena);
		// Recalculate allocation with new block
		insert = (u8*)insert + size + GetAlignmentPadding(insert, align);
		++freeBlock->count;
		return (u8*)insert - size;
	}

	template<sizet blockSize>
	void Details::LinearBasePool<blockSize>::Free(Arena& parentArena, void* ptr, sizet size)
	{
		auto* block = static_cast<LinearBlock*>(GetAlignedBlock(ptr, GetBlockSize()));
		if (!block) [[unlikely]]
		{
			return;
		}

		--block->count;
		if (block->count <= 0) [[unlikely]]
		{
			// If the block is empty and was marked full, free it
			FreeBlock(parentArena, block);
		}
	}

	template<sizet blockSize>
	void Details::LinearBasePool<blockSize>::Release(Arena& parentArena)
	{
		// Iterate backwards all blocks while freeing them
		insert = nullptr;
		while (freeBlock != nullptr)
		{
			LinearBlock* const block = freeBlock;
			freeBlock                = freeBlock->last;
			parentArena.Free(block->unaligned, GetAllocatedBlockSize());
		}
		freeBlock = nullptr;
	}


	MultiLinearArena::MultiLinearArena(Arena& parentArena) : ChildArena(&parentArena)
	{
		Interface<MultiLinearArena>();
	}

	void* MultiLinearArena::Alloc(sizet size)
	{
		// TODO: Resolve a reasonable align size based on allocation size
		return Alloc(size, alignof(std::max_align_t));
	}

	void* MultiLinearArena::Alloc(sizet size, sizet align)
	{
		if (size < smallPool.maxSize)
		{
			return smallPool.Alloc(GetParentArena(), size, align);
		}
		else if (size < mediumPool.maxSize)
		{
			return mediumPool.Alloc(GetParentArena(), size, align);
		}
		else if (size < bigPool.maxSize)
		{
			return bigPool.Alloc(GetParentArena(), size, align);
		}
		else
		{
			return GetParentArena().Alloc(size, align);
		}
	}

	void MultiLinearArena::Free(void* ptr, sizet size)
	{
		if (size < smallPool.maxSize)
		{
			smallPool.Free(GetParentArena(), ptr, size);
		}
		else if (size < mediumPool.maxSize)
		{
			mediumPool.Free(GetParentArena(), ptr, size);
		}
		else if (size < bigPool.maxSize)
		{
			bigPool.Free(GetParentArena(), ptr, size);
		}
		else
		{
			GetParentArena().Free(ptr, size);
		}
	}

	void MultiLinearArena::Release()
	{
		smallPool.Release(GetParentArena());
		mediumPool.Release(GetParentArena());
		bigPool.Release(GetParentArena());
	}
#pragma endregion Multi Linear

#pragma region Best Fit Arena
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
		Interface<BestFitArena>();

		P_Check(initialSize > 0);
		block.data = GetParentArena().Alloc(initialSize);
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
			if ((float(freeSlots.Size()) / freeSlots.Capacity()) < 0.1f)
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
#pragma endregion Best Fit Arena

#pragma region Big Best Fit Arena
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
		Interface<BigBestFitArena>();

		P_Check(initialSize > 0);
		block.data = GetParentArena().Alloc(initialSize);
		block.size = initialSize;
		// Set address at end of block. Size is 0
		// freeSlots.SetData(static_cast<u8*>(block.GetData()) + block.Size());
		// Add first slot for the entire block
		freeSlots.Add({0, u32(block.size)});

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
		alignment = Max(alignment, minAlignment);

		const i32 slotIndex = FindSmallestSlot(size + alignment - 1);
		if (slotIndex == NO_INDEX || slotIndex >= freeSlots.Size())
		{
			// Error("No slots can fit {} bytes!", size);
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
#pragma endregion Big Best Fit Arena
}    // namespace p
