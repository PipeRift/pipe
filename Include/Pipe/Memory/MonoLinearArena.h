// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Limits.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"
#include "Pipe/PipeArrays.h"


namespace p
{
	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class PIPE_API MonoLinearArena : public ChildArena
	{
	protected:
		void* insert = nullptr;
		sizet count  = 0;
		Memory::Block block{};
		bool selfAllocated = false;


	public:
		MonoLinearArena(Memory::Block externalBlock, Arena& parentArena = GetCurrentArena())
		    : ChildArena(&parentArena), insert{externalBlock.data}, block{Move(externalBlock)}
		{
			Interface<MonoLinearArena>();
		}
		MonoLinearArena(const sizet blockSize = Memory::MB, Arena& parentArena = GetCurrentArena())
		    : ChildArena(&parentArena)
		    , insert{GetParentArena().Alloc(blockSize)}
		    , block{insert, blockSize}
		    , selfAllocated{true}
		{
			Interface<MonoLinearArena>();
		}
		~MonoLinearArena() override
		{
			Release(false);
		}

		void* Alloc(sizet size)
		{
			// TODO: Resolve a reasonable align size based on allocation size
			return Alloc(size, alignof(std::max_align_t));
		}
		void* Alloc(sizet size, sizet align);
		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		void Release(bool keepIfSelfAllocated = true);


		sizet GetUsedMemory() const override
		{
			return (u8*)insert - (u8*)block.data;
		}
		sizet GetAvailableMemory() const override
		{
			return block.size;
		}
		void GetBlocks(TArray<Memory::Block>& outBlocks) const override
		{
			outBlocks.Add(block);
		}
	};

	// TMonoLinearArena works like a MonoLinearArena but providing a block on the stack as the block
	// to use
	template<sizet blockSize = Memory::MB>
	class PIPE_API TMonoLinearArena : public MonoLinearArena
	{
		u8 buffer[blockSize];


		TMonoLinearArena(Arena& parentArena = GetCurrentArena())
		    : MonoLinearArena(Memory::Block{buffer, blockSize}, parentArena)
		{}
	};
}    // namespace p
