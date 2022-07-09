// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/IAllocator.h"


namespace p
{
	class ArenaAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
			Arena* arena = nullptr;

		public:

			Typed() : arena{GetCurrentArena()}
			{
				CheckMsg(arena, "No current arena set");
			}

			T* Alloc(const sizet count)
			{
				return p::Alloc<T>(*arena, count);
			}

			T* Alloc(const sizet count, const sizet align)
			{
				return p::Alloc<T>(*arena, count, align);
			}

			bool Resize(T* ptr, sizet ptrCount, sizet count)
			{
				return p::Resize<T>(*arena, ptr, ptrCount, count);
			}

			void Free(T* ptr, sizet count)
			{
				p::Free<T>(*arena, ptr, count);
			}
		};
	};
}    // namespace p
