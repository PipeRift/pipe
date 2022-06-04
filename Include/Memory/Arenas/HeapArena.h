// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"
#include "Memory/Arenas/IArena.h"


namespace Pipe::Memory
{
	class CORE_API NativeArena : public IArena
	{
	public:
		NativeArena()          = default;
		virtual ~NativeArena() = default;

		void* Allocate(const sizet size)
		{
			return Pipe::Alloc(size);
		}
		void* Allocate(const sizet size, const sizet align)
		{
			return Pipe::Alloc(size, align);
		}

		void Free(void* ptr)
		{
			Pipe::Free(ptr);
		}
	};
}    // namespace Pipe::Memory
