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
		// TODO: Windows needs _aligned_free in order to use _aligned_alloc()
		void* const p = std::malloc(n);
#elif PLATFORM_MACOS || PLATFORM_LINUX
		void* p;
		posix_memalign(&p, align, n);
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