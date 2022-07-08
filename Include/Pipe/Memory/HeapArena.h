// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/IArena.h"


namespace p
{
	class PIPE_API HeapArena : public IArena
	{
	public:
		HeapArena()
		{
			SetupInterface(
			    &HeapArena::Alloc, &HeapArena::Alloc, &HeapArena::Resize, &HeapArena::Free);
		}
		~HeapArena() override = default;

		void* Alloc(const sizet size)
		{
			return p::Alloc(size);
		}
		void* Alloc(const sizet size, const sizet align)
		{
			return p::Alloc(size, align);
		}
		bool Resize(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr)
		{
			p::Free(ptr);
		}
	};
}    // namespace p
