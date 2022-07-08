// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/HeapAllocator.h"
#include "Pipe/Memory/IAllocator.h"


namespace p
{
	template<sizet N, typename FallbackAllocator = HeapAllocator>
	class TStackAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			bool inUse = false;
			T buffer[N];

			typename FallbackAllocator::Typed<T> fallback;


			T* Alloc(const sizet count)
			{
				return static_cast<T*>(p::Alloc(count * sizeof(T)));
			}
			T* Alloc(const sizet count, const sizet align)
			{
				return static_cast<T*>(p::Alloc(count * sizeof(T), align));
			}

			void Free(T* ptr)
			{
				p::Free(ptr);
			}
		};
	};
}    // namespace p
