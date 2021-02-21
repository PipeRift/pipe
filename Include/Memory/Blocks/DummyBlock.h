// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseBlock.h"


namespace Rift::Memory
{
	/** This serves as a memory Block interface.
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

		// Get memory and size
		const void* GetData() const
		{
			return nullptr;
		}
		void* GetData()
		{
			return nullptr;
		}
		sizet GetSize() const
		{
			return 0;
		}

		bool IsAllocated() const
		{
			// return !!data;
			return false;
		}
	};
}    // namespace Rift::Memory
