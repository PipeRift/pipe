// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"
#include "Memory/Arenas/IArena.h"


namespace pipe::Memory
{
	class CORE_API NativeArena : public IArena
	{
	public:
		NativeArena()          = default;
		virtual ~NativeArena() = default;

		void* Allocate(const sizet size)
		{
			return pipe::Alloc(size);
		}
		void* Allocate(const sizet size, const sizet align)
		{
			return pipe::Alloc(size, align);
		}

		void Free(void* ptr)
		{
			pipe::Free(ptr);
		}
	};
}    // namespace pipe::Memory
