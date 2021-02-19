// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Alloc.h"

#include "Profiler.h"

#include <cstdlib>
#include <memory>


namespace Rift
{
	void* Alloc(size_t n)
	{
		void* const p = std::malloc(n);
		TracyAllocS(p, n, 8);
		return p;
	}

	void* Alloc(size_t n, size_t align)
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
		TracyAllocS(p, n, 8);
		return p;
	}

	void Free(void* p)
	{
		TracyFreeS(p, 8);
		std::free(p);
	}
}    // namespace Rift
