// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseBlock.h"


namespace Rift::Memory
{
	class CORE_API DummyBlock : BaseBlock
	{
	public:
		DummyBlock()  = default;
		~DummyBlock() = default;
		// Implement copy or move as desired

		// Allocate the block of memory with an optional size
		void Allocate(size_t) {}
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
		size_t GetSize() const
		{
			return 0;
		}
	};
}    // namespace Rift::Memory
