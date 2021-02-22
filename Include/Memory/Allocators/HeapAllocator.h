// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift::Memory
{
	/** This is an Allocator interface.
	 * It serves the single purpose of documenting an Allocator's interface.
	 * Not intended to be used.
	 */
	class CORE_API HeapAllocator
	{
	public:
		HeapAllocator() {}
		~HeapAllocator() {}

		void* Allocate(const sizet size)
		{
			return Rift::Alloc(size);
		}
		void* Allocate(const sizet size, const sizet align)
		{
			return Rift::Alloc(size, align);
		}

		void Free(void* ptr)
		{
			Rift::Free(ptr);
		}
	};
}    // namespace Rift::Memory
