// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/IAllocator.h"


namespace p
{
	class HeapAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			T* Alloc(const sizet count)
			{
				return static_cast<T*>(p::Alloc(count * sizeof(T)));
			}

			T* Alloc(const sizet count, const sizet align)
			{
				return static_cast<T*>(p::Alloc(count * sizeof(T), align));
			}

			T* Realloc(T* ptr, sizet ptrSize, sizet size)
			{
				return static_cast<T*>(p::Realloc(ptr, ptrSize * sizeof(T), size * sizeof(T)));
			}

			void Free(T* ptr, sizet size)
			{
				p::Free(ptr, size);
			}
		};
	};
}    // namespace p
