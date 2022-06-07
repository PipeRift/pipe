// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arenas/IArena.h"


namespace p::Memory
{
	class PIPE_API NativeArena : public IArena
	{
	public:
		NativeArena()          = default;
		virtual ~NativeArena() = default;

		void* Allocate(const sizet size)
		{
			return p::Alloc(size);
		}
		void* Allocate(const sizet size, const sizet align)
		{
			return p::Alloc(size, align);
		}

		void Free(void* ptr)
		{
			p::Free(ptr);
		}
	};
}    // namespace p::Memory
