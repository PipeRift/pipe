// Copyright 2015-2022 Piperift - All rights reserved
#pragma once


#include "Pipe/Core/Platform.h"


namespace p
{
	class PIPE_API IAllocator
	{};

	class DummyAllocator : public IAllocator
	{
	public:
		template<typename T>
		class Typed
		{
		public:
			T* Alloc(sizet size)
			{
				return nullptr;
			}
			T* Alloc(sizet size, const sizet align)
			{
				return nullptr;
			}
			T* Realloc(T* ptr, sizet ptrSize, sizet size)
			{
				return nullptr;
			}

			void Free(T* ptr, sizet size) {}
		};
	};
}    // namespace p
