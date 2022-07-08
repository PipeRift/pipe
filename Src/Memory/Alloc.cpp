// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/Alloc.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Profiler.h"

#include <cstdlib>
#include <memory>


namespace p
{
	void* Alloc(sizet n)
	{
		void* const p = std::malloc(n);
		// FIX: Profiler reports alloc gets called frequently twice with the same pointer. Seems
		// related to allocators
		// TracyAllocS(p, n, 12);
		return p;
	}

	void* Alloc(sizet n, sizet align)
	{
#if PLATFORM_WINDOWS
		// TODO: Windows needs _aligned_free in order to use _aligned_alloc()
		void* const p = std::malloc(n);
#elif PLATFORM_MACOS || PLATFORM_LINUX
		void* p;
		(void)(posix_memalign(&p, align, n));
#else
		void* const p = std::aligned_alloc(align, n);
#endif
		// TracyAllocS(p, n, 8);
		return p;
	}

	void* Realloc(void* old, sizet size)
	{
		// TracyFreeS(old, 8);
		void* const p = std::realloc(old, size);
		// TracyAllocS(p, size, 8);
		return p;
	}

	void Free(void* p)
	{
		// TracyFreeS(p, 8);
		std::free(p);
	}
}    // namespace p
