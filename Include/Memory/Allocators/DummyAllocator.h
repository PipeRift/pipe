// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift::Memory
{
	/** This is an Allocator interface.
	 * It serves the single purpose of documenting an Allocator's interface.
	 * Not intended to be used.
	 */
	class CORE_API DummyAllocator
	{
	public:
		DummyAllocator() {}
		~DummyAllocator() {}

		// Define copy or move as desired
		DummyAllocator(const DummyAllocator&) = default;
		DummyAllocator(DummyAllocator&&)      = default;
		DummyAllocator& operator=(const DummyAllocator&) = default;
		DummyAllocator& operator=(DummyAllocator&&) = default;

		void* Allocate(const sizet size)
		{
			return nullptr;
		}
		void* Allocate(const sizet size, const sizet alignment)
		{
			return nullptr;
		}
		void Free(void* ptr) {}
	};
}    // namespace Rift::Memory
