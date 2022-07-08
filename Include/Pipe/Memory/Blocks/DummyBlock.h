// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseBlock.h"


namespace p::Memory
{
	/** This is a memory Block interface.
	 * It serves the single purpose of documenting a Block's interface.
	 * Not intended to be used.
	 */
	class PIPE_API DummyBlock : BaseBlock
	{
	public:
		DummyBlock()  = default;
		~DummyBlock() = default;
		// Implement copy or move as desired

		// Allocate the block of memory with an optional size
		void Alloc(sizet) {}
		// Free the block of memory
		void Free() {}
	};
}    // namespace p::Memory
