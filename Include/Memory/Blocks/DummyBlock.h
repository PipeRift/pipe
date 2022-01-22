// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseBlock.h"


namespace Rift::Memory
{
	/** This is a memory Block interface.
	 * It serves the single purpose of documenting a Block's interface.
	 * Not intended to be used.
	 */
	class CORE_API DummyBlock : BaseBlock
	{
	public:
		DummyBlock()  = default;
		~DummyBlock() = default;
		// Implement copy or move as desired

		// Allocate the block of memory with an optional size
		void Allocate(sizet) {}
		// Free the block of memory
		void Free() {}
	};
}    // namespace Rift::Memory
