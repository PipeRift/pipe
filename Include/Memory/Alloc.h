// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift
{
	CORE_API void* Alloc(sizet n);
	CORE_API void* Alloc(sizet n, sizet align);
	CORE_API void* Realloc(void* old, sizet newSize);
	CORE_API void Free(void* p);

	/**
	 * @return the number of bytes needed for p to be aligned in 'align'
	 */
	CORE_API sizet GetAlignmentPadding(const void* p, sizet align);

	CORE_API sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize);
}    // namespace Rift
