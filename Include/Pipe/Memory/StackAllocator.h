// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/HeapAllocator.h"
#include "Pipe/Memory/IAllocator.h"


namespace p
{
	template<sizet N, typename FallbackAllocator = HeapAllocator>
	class TInlineAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			bool inUse = false;
			T buffer[N];

			typename FallbackAllocator::Typed<T> fallback;


			T* Alloc(sizet size)
			{
				if (inUse)
				{
					return fallback.Alloc(size);
				}
				inUse = true;
				return buffer;
			}
			T* Alloc(sizet size, sizet align)
			{
				if (inUse)
				{
					return fallback.Alloc(size, align);
				}
				inUse = true;
				return buffer;
			}

			void Free(T* ptr, sizet n)
			{
				if (ptr == buffer)
				{
					inUse = false;
				}
				else
				{
					fallback.Free(ptr, n);
				}
			}

			T* Realloc(T* ptr, sizet ptrSize, sizet size)
			{
				if (size <= N)
				{
					if (ptr != buffer)    // Previous ptr was allocated on fallback
					{
						MoveConstructItems<T>(buffer, ptr, ptrSize);
						fallback.Free(ptr, ptrSize);
						inUse = true;
					}
					return buffer;
				}
				else
				{
					if (ptr == buffer)    // Previous ptr was allocated inline
					{
						ptr = fallback.Alloc(size);
						MoveConstructItems<T>(ptr, buffer, ptrSize);
						inUse = false;
						return ptr;
					}

					// Completely external reallocation
					return fallback.Realloc(ptr, ptrSize, size);
				}
			}
		};
	};
}    // namespace p
