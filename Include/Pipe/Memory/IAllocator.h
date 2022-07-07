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
			T* Alloc(const sizet n)
			{
				return nullptr;
			}
			T* Alloc(const sizet n, const sizet align)
			{
				return nullptr;
			}

			void Free(T* ptr) {}
		};
	};
}    // namespace p
