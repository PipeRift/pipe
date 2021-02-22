// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift
{
	CORE_API void* Alloc(sizet n);

	CORE_API void* Alloc(sizet n, sizet align);

	CORE_API void Free(void* p);

	/**
	 * @return the number of bytes needed for p to be aligned in 'alignment'
	 */
	CORE_API sizet GetAlignmentPadding(void* p, sizet align);
}    // namespace Rift
