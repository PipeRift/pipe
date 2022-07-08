// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	PIPE_API void* Alloc(sizet n);
	PIPE_API void* Alloc(sizet n, sizet align);
	PIPE_API void* Realloc(void* old, sizet newSize);
	PIPE_API void Free(void* p);
}    // namespace p
