// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include <Tracy.hpp>
#include <cstdlib>
#include <memory>


namespace Rift
{
	static inline void* Alloc(size_t n)
	{
		void* const p = std::malloc(n);
		TracyAlloc(p, n);
		return p;
	}

	static inline void* Alloc(size_t n, size_t align)
	{
#if PLATFORM_WINDOWS
		void* const p = _aligned_malloc(align, n);
#else
		void* const p = std::aligned_alloc(align, n);
#endif
		TracyAlloc(p, n);
		return p;
	}

	static inline void Free(void* p)
	{
		TracyFree(p);
		std::free(p);
	}
}	 // namespace Rift