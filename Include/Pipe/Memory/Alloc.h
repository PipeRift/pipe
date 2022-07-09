// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	class Arena;

	// Native allocation functions
	PIPE_API void* HeapAlloc(sizet size);
	PIPE_API void* HeapAlloc(sizet size, sizet align);
	PIPE_API void* HeapRealloc(void* ptr, sizet size);
	PIPE_API void HeapFree(void* ptr);


	PIPE_API class HeapArena& GetHeapArena();
	PIPE_API Arena* GetCurrentArena();
	PIPE_API void SetCurrentArena(Arena& arena);

	// Arena allocation functions
	PIPE_API void* Alloc(Arena& arena, sizet size);
	PIPE_API void* Alloc(Arena& arena, sizet size, sizet align);
	PIPE_API bool Resize(Arena& arena, void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(Arena& arena, void* ptr, sizet size);
	PIPE_API void* Alloc(sizet size);
	PIPE_API void* Alloc(sizet size, sizet align);
	PIPE_API bool Resize(void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(void* ptr, sizet size);

	template<typename T>
	T* Alloc(Arena& arena, sizet count)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, alignof(T)));
	}
	template<typename T>
	T* Alloc(Arena& arena, sizet count, sizet align)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, align));
	}
	template<typename T>
	bool Resize(Arena& arena, T* ptr, sizet ptrCount, sizet count)
	{
		return Resize(arena, ptr, sizeof(T) * ptrCount, sizeof(T) * count);
	}
	template<typename T>
	void Free(Arena& arena, T* ptr, u32 count)
	{
		Free(arena, static_cast<void*>(ptr), sizeof(T) * count);
	}
}    // namespace p
