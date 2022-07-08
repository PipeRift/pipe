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
			static T* Alloc(const sizet count)
			{
				return static_cast<T*>(p::Alloc(count * sizeof(T), alignof(T)));
			}

			static T* Alloc(const sizet count, const sizet align)
			{
				return static_cast<T*>(p::Alloc(count * sizeof(T), align));
			}

			static bool Resize(T* ptr, sizet ptrSize, sizet size)
			{
				return false;    // TODO
			}

			static void Free(T* ptr, sizet size)
			{
				p::Free(ptr);
			}
		};
	};
}    // namespace p
