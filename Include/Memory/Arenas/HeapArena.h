// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"
#include "Memory/Arenas/IArena.h"


namespace Rift::Memory
{
	class CORE_API HeapArena : public IArena
	{
	public:
		HeapArena()          = default;
		virtual ~HeapArena() = default;

		void* Allocate(const sizet size)
		{
			return Rift::Alloc(size);
		}
		void* Allocate(const sizet size, const sizet align)
		{
			return Rift::Alloc(size, align);
		}

		void Free(void* ptr)
		{
			Rift::Free(ptr);
		}
	};
}    // namespace Rift::Memory
