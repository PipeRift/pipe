// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace pipe::Memory
{
	/** IArena defines the API used on all other arena types */
	class CORE_API IArena
	{
	protected:
		IArena() {}

	public:
		virtual ~IArena() {}
		virtual void* Allocate(sizet size)                  = 0;
		virtual void* Allocate(sizet size, sizet alignment) = 0;
		virtual void Free(void* ptr, sizet size)            = 0;
		// Reallocate allows for optimizations between free and alocate
		virtual void* Reallocate(void* oldPtr, sizet oldSize, sizet newSize, sizet alignment = 0)
		{
			Free(oldPtr, oldSize);
			return Allocate(newSize, alignment);
		}


		/* Helpers */

		template<typename T>
		T* Allocate()
		{
			return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
		}

		template<typename T>
		T* AllocateArray(u32 count)
		{
			return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
		}

		template<typename T>
		void Free(T* ptr)
		{
			Free(ptr, sizeof(T));
		}

		template<typename T>
		void FreeArray(T* ptr, u32 count)
		{
			Free(ptr, sizeof(T) * count);
		}
	};
}    // namespace pipe::Memory
