// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	PIPE_API void* Alloc(sizet size);
	PIPE_API void* Alloc(sizet size, sizet align);
	PIPE_API void* Realloc(void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(void* ptr, sizet size);
}    // namespace p
