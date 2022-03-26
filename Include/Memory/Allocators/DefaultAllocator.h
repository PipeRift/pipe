// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Allocators/IAllocator.h"


namespace Rift::Memory
{
	template<typename T>
	class TDefaultAllocator : public IAllocator
	{
	public:
		template<typename U>
		using Rebind = TDefaultAllocator<U>;


		T* Allocate(const sizet count)
		{
			return static_cast<T*>(Rift::Alloc(count * sizeof(T)));
		}
		T* Allocate(const sizet count, const sizet align)
		{
			return static_cast<T*>(Rift::Alloc(count * sizeof(T), align));
		}

		void Free(T* ptr)
		{
			Rift::Free(ptr);
		}
	};
}    // namespace Rift::Memory
