// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	class IArena;

	// Native allocation functions
	PIPE_API void* Alloc(sizet size);
	PIPE_API void* Alloc(sizet size, sizet align);
	PIPE_API void* Realloc(void* ptr, sizet size);
	PIPE_API void Free(void* ptr);

	// Arena allocation functions
	PIPE_API IArena* GetGlobalArena();
	PIPE_API void* Alloc(IArena* arena, sizet size);
	PIPE_API void* Alloc(IArena* arena, sizet size, sizet align);
	PIPE_API bool Resize(IArena* arena, void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(IArena* arena, void* ptr, sizet size);

	template<typename T>
	T* Alloc(IArena* arena)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T), alignof(T)));
	}

	template<typename T>
	T* AllocMany(IArena* arena, u32 count)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, alignof(T)));
	}

	template<typename T>
	void Free(IArena* arena, T* ptr)
	{
		Free(arena, ptr);
	}

	template<typename T>
	void FreeMany(IArena* arena, T* ptr, u32 count)
	{
		Free(arena, ptr);
	}
}    // namespace p
