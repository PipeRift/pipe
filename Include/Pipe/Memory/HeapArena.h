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
			Interface<HeapArena, &HeapArena::Alloc, &HeapArena::Alloc, &HeapArena::Resize,
			    &HeapArena::Free>();
		}
		~HeapArena() override = default;

		inline void* Alloc(const sizet size)
		{
			return p::HeapAlloc(size);
		}
		inline void* Alloc(const sizet size, const sizet align)
		{
			return p::HeapAlloc(size, align);
		}
		inline bool Resize(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		inline void Free(void* ptr, sizet size)
		{
			p::HeapFree(ptr);
		}
	};
}    // namespace p
