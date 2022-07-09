// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Memory/IAllocator.h"

#include <cstdlib>


namespace p
{
	// Use a custom allocator that doesn't track allocations. Otherwise tracking stats would loop
	class MemoryStatsAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			T* Alloc(const sizet count)
			{
				return static_cast<T*>(std::malloc(sizeof(T) * count));
			}

			T* Alloc(const sizet count, const sizet align)
			{
				return static_cast<T*>(std::aligned_alloc(align, sizeof(T) * count));
			}

			bool Resize(T* ptr, sizet ptrCount, sizet count)
			{
				return false;
			}

			void Free(T* ptr, sizet count)
			{
				std::free(ptr);
			}
		};
	};

	struct PIPE_API MemoryStats
	{
		sizet used = 0;

		TMap<void*, sizet, MemoryStatsAllocator> trackedAllocations;


		void Add(void* ptr, sizet size)
		{
			used += size;
			trackedAllocations.Insert(ptr, size);
		}

		void Remove(void* ptr)
		{
			auto it = trackedAllocations.FindIt(ptr);
			if (it != trackedAllocations.end())
			{
				used -= it->second;
				trackedAllocations.RemoveIt(it);
			}
		}
	};
}    // namespace p
