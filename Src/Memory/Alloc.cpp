// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Alloc.h"

#include "Math/Math.h"
#include "Misc/Checks.h"
#include "Profiler.h"

#include <cstdlib>
#include <memory>



namespace Rift
{
	void* Alloc(sizet n)
	{
		void* const p = std::malloc(n);
		TracyAllocS(p, n, 8);
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
		TracyAllocS(p, n, 8);
		return p;
	}

	void* Realloc(void* old, sizet size)
	{
		TracyFreeS(old, 8);
		void* const p = std::realloc(old, size);
		TracyAllocS(p, size, 8);
		return p;
	}

	void Free(void* p)
	{
		TracyFreeS(p, 8);
		std::free(p);
	}

	sizet GetAlignmentPadding(const void* ptr, sizet align)
	{
		Check(Math::IsPowerOfTwo(align));
		return -reinterpret_cast<ssizet>(ptr) & (align - 1);
	}

	sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize)
	{
		// Get padding with the header as an offset
		return headerSize + GetAlignmentPadding(static_cast<const u8*>(ptr) + headerSize, align);
	}
}    // namespace Rift
