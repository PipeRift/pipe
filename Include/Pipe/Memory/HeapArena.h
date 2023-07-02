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
			Interface<HeapArena, &HeapArena::Alloc, &HeapArena::Alloc, &HeapArena::Realloc,
			    &HeapArena::Free>();
		}
		~HeapArena() override = default;

		constexpr void* Alloc(const sizet size)
		{
			return p::HeapAlloc(size);
		}
		constexpr void* Alloc(const sizet size, const sizet align)
		{
			return p::HeapAlloc(size, align);
		}
		inline bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		constexpr void Free(void* ptr, sizet size)
		{
			p::HeapFree(ptr);
		}
	};
}    // namespace p
