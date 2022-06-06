// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace p
{
	PIPE_API void* Alloc(sizet n);
	PIPE_API void* Alloc(sizet n, sizet align);
	PIPE_API void* Realloc(void* old, sizet newSize);
	PIPE_API void Free(void* p);

	/**
	 * @return the number of bytes needed for p to be aligned in 'align'
	 */
	PIPE_API sizet GetAlignmentPadding(const void* p, sizet align);

	PIPE_API sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize);
}    // namespace p
