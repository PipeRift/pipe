// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"


namespace p
{
	class PIPE_API HeapArena : public Arena
	{
	public:
		~HeapArena() override = default;

		void* Alloc(const sizet size) override
		{
			return p::HeapAlloc(size);
		}
		void* Alloc(const sizet size, const sizet align) override
		{
			return p::HeapAlloc(size, align);
		}
		bool Resize(void* ptr, const sizet ptrSize, const sizet size) override
		{
			return false;
		}
		void Free(void* ptr, sizet size) override
		{
			p::HeapFree(ptr);
		}
	};
}    // namespace p
