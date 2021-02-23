// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift::Memory
{
	class CORE_API IAllocator
	{
	public:
		enum
		{
			// Allocator flag defaults
			NeedsElementType = false
		};

	protected:
		IAllocator() = default;

	public:
		virtual ~IAllocator() = default;

		virtual void* Allocate(const sizet size)                    = 0;
		virtual void* Allocate(const sizet size, const sizet align) = 0;

		virtual void Free(void* ptr) = 0;
	};
}    // namespace Rift::Memory
