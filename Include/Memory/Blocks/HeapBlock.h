// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"
#include "Memory/Blocks/Block.h"


namespace p::Memory
{
	class PIPE_API HeapBlock : public Block
	{
	public:
		HeapBlock() = default;
		HeapBlock(sizet initialSize);
		~HeapBlock();
		HeapBlock(const HeapBlock& other);
		HeapBlock(HeapBlock&& other);
		HeapBlock& operator=(const HeapBlock& other);
		HeapBlock& operator=(HeapBlock&& other);

		void Allocate(sizet size);
		void Free();
	};
}    // namespace p::Memory
