// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"

#include "Pipe/Memory/MemoryStats.h"


namespace p
{
	class PIPE_API HeapArena : public Arena
	{
		MemoryStats stats;

	public:
		HeapArena()
		{
			Interface<HeapArena>();
		}
		~HeapArena() override = default;

		void* Alloc(const sizet size)
		{
			void* ptr = p::HeapAlloc(size);
			stats.Add(ptr, size);
			return ptr;
		}
		void* Alloc(const sizet size, const sizet align)
		{
			void* ptr = p::HeapAlloc(size, align);
			stats.Add(ptr, size);
			return ptr;
		}
		bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size)
		{
			stats.Remove(ptr);
			p::HeapFree(ptr);
		}

		const MemoryStats& GetStats() const { return stats; }
	};
}    // namespace p
