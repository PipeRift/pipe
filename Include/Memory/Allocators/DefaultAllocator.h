// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Allocators/IAllocator.h"


namespace Rift::Memory
{
	class CORE_API DefaultAllocator : public IAllocator
	{
	public:
		DefaultAllocator()          = default;
		virtual ~DefaultAllocator() = default;

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


	template <u32 numElements>
	class InlineAllocator : public IAllocator
	{
	public:
		enum
		{
			NeedsElementType = true
		};


	public:
		InlineAllocator()          = default;
		virtual ~InlineAllocator() = default;

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
