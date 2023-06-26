// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	class Arena;
	struct MemoryStats;


	// Native allocation functions
	PIPE_API void* HeapAlloc(sizet size);
	PIPE_API void* HeapAlloc(sizet size, sizet align);
	PIPE_API void* HeapRealloc(void* ptr, sizet size);
	PIPE_API void HeapFree(void* ptr);


	PIPE_API class HeapArena& GetHeapArena();
	PIPE_API Arena& GetCurrentArena();
	PIPE_API void SetCurrentArena(Arena& arena);
	PIPE_API MemoryStats* GetHeapStats();

	// Arena allocation functions (Find current arena)
	PIPE_API void* Alloc(sizet size);
	PIPE_API void* Alloc(sizet size, sizet align);
	PIPE_API bool Realloc(void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(void* ptr, sizet size);

	// Arena allocation functions (Use specific arena)
	template<Derived<Arena> ArenaT>
	void* Alloc(ArenaT& arena, sizet size)
	{
		return arena.Alloc(size);
	}
	template<Derived<Arena> ArenaT>
	void* Alloc(ArenaT& arena, sizet size, sizet align)
	{
		return arena.Alloc(size, align);
	}
	template<Derived<Arena> ArenaT>
	bool Realloc(ArenaT& arena, void* ptr, sizet ptrSize, sizet newSize)
	{
		return arena.Realloc(ptr, ptrSize, newSize);
	}
	template<Derived<Arena> ArenaT>
	void Free(ArenaT& arena, void* ptr, sizet size)
	{
		arena.Free(ptr, size);
	}


	// Templated arena allocation functions:

	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count = 1) requires(!IsVoid<T>)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, alignof(T)));
	}
	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count, sizet align) requires(!IsVoid<T>)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, align));
	}
	template<typename T, Derived<Arena> ArenaT>
	bool Realloc(ArenaT& arena, T* ptr, sizet ptrCount, sizet newCount) requires(!IsVoid<T>)
	{
		return Realloc(arena, ptr, sizeof(T) * ptrCount, sizeof(T) * newCount);
	}
	template<typename T, Derived<Arena> ArenaT>
	void Free(ArenaT& arena, T* ptr, u32 count = 1) requires(!IsVoid<T>)
	{
		Free(arena, static_cast<void*>(ptr), sizeof(T) * count);
	}
}    // namespace p
