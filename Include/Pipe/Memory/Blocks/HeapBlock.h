// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Blocks/Block.h"


namespace p::Memory
{
	class PIPE_API HeapBlock : public Block
	{
	public:
		HeapBlock() = default;
		HeapBlock(sizet initialSize);
		~HeapBlock();
		HeapBlock(const HeapBlock& other);
		HeapBlock(HeapBlock&& other) noexcept;
		HeapBlock& operator=(const HeapBlock& other);
		HeapBlock& operator=(HeapBlock&& other) noexcept;

		void Allocate(sizet size);
		void Free();
	};
}    // namespace p::Memory
