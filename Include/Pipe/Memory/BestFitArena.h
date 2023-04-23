// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	class PIPE_API BestFitArena : public ChildArena
	{
	public:
		struct Slot
		{
			u8* start;
			sizet size;

			u8* End() const;

			auto operator==(const Slot& other) const
			{
				return size == other.size;
			}
			auto operator<(const Slot& other) const
			{
				return size < other.size;
			}
			auto operator>(const Slot& other) const
			{
				return size > other.size;
			}
			auto operator<=(const Slot& other) const
			{
				return size <= other.size;
			}
			auto operator>=(const Slot& other) const
			{
				return size >= other.size;
			}
		};

	protected:
		// TODO: Support growing multiple blocks
		Memory::Block block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BestFitArena(Arena* parent, const sizet initialSize = 4 * Memory::KB);
		BestFitArena(const sizet initialSize = 4 * Memory::KB) : BestFitArena(nullptr, initialSize)
		{}
		~BestFitArena() override;

		void* Alloc(const sizet size);
		void* Alloc(const sizet size, sizet align);
		bool Resize(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		const Memory::Block& GetBlock() const
		{
			return block;
		}

		bool Contains(void* ptr)
		{
			return block.Contains(ptr);
		}
		sizet GetFreeSize()
		{
			return freeSize;
		}

		sizet GetUsedSize()
		{
			return block.size - freeSize;
		}

		const TArray<Slot>& GetFreeSlots() const
		{
			return freeSlots;
		}

	private:
		i32 FindSmallestSlot(sizet neededSize);
		void ReduceSlot(
		    i32 slotIndex, Slot& slot, u8* const allocationStart, u8* const allocationEnd);
		void AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd);
	};


	inline u8* BestFitArena::Slot::End() const
	{
		return start + size;
	}
}    // namespace p
