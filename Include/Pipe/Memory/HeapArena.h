// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"


namespace p
{
	class PIPE_API HeapArena : public Arena
	{
	public:
		HeapArena()
		{
			Interface<HeapArena>();
		}
		~HeapArena() override = default;

		void* Alloc(const sizet size)
		{
			return p::HeapAlloc(size);
		}
		void* Alloc(const sizet size, const sizet align)
		{
			return p::HeapAlloc(size, align);
		}
		bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size)
		{
			p::HeapFree(ptr);
		}
	};
}    // namespace p
