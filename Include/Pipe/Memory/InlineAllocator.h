// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/ArenaAllocator.h"
#include "Pipe/Memory/IAllocator.h"


namespace p
{
	template<sizet N, typename FallbackAllocator = ArenaAllocator>
	class TInlineAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			bool used = false;
			T buffer[N];

			typename FallbackAllocator::template Typed<T> fallback;


			T* Alloc(sizet size)
			{
				if (used)
				{
					return fallback.Alloc(size);
				}
				used = true;
				return buffer;
			}
			T* Alloc(sizet size, sizet align)
			{
				if (used)
				{
					return fallback.Alloc(size, align);
				}
				used = true;
				return buffer;
			}

			void Free(T* ptr, sizet n)
			{
				if (ptr == buffer)
				{
					used = false;
				}
				else
				{
					fallback.Free(ptr, n);
				}
			}

			bool Resize(T* ptr, sizet ptrSize, sizet size)
			{
				if (size <= N)
				{
					// if ptr is allocated externally and new size is smaller than
					// inline buffer, don't allow Resize
					return ptr == buffer;
				}
				else if (ptr == buffer)
				{
					// If ptr is allocated inlined but new size exceeds inline
					// buffer, can't Resize
					return false;
				}
				return fallback.Resize(ptr, ptrSize, size);
			}
		};
	};
}    // namespace p
