// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift
{
	CORE_API void* Alloc(size_t n);

	CORE_API void* Alloc(size_t n, size_t align);

	CORE_API void Free(void* p);
}	 // namespace Rift