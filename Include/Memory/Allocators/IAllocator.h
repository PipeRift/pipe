// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift::Memory
{
	class CORE_API IAllocator
	{
	public:
		// Allocator flag defaults
		enum
		{
			// If true, subclass Typed<T> will be used as the allocator instead of the allocator
			UsesTemplateType = false
		};

	protected:
		IAllocator() = default;
	};


	class DummyAllocator : public IAllocator
	{
	public:
		enum
		{
			NeedsElementType = false
		};

		DummyAllocator() = default;

		void* Allocate(const sizet size)
		{
			return nullptr;
		}
		void* Allocate(const sizet size, const sizet align)
		{
			return nullptr;
		}

		void Free(void* ptr) {}
	};
}    // namespace Rift::Memory
