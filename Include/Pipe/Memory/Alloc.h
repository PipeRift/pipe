// Copyright 2015-2022 Piperift - All rights reserved
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
	PIPE_API Arena* GetCurrentArena();
	PIPE_API void SetCurrentArena(Arena& arena);
	PIPE_API MemoryStats* GetHeapStats();

	// Arena allocation functions
	PIPE_API void* Alloc(sizet size);
	PIPE_API void* Alloc(sizet size, sizet align);
	PIPE_API bool Resize(void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(void* ptr, sizet size);
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
	bool Resize(ArenaT& arena, void* ptr, sizet ptrSize, sizet size)
	{
		return arena.Resize(ptr, ptrSize, size);
	}
	template<Derived<Arena> ArenaT>
	void Free(ArenaT& arena, void* ptr, sizet size)
	{
		arena.Free(ptr, size);
	}

	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count) requires(!IsVoid<T>)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, alignof(T)));
	}
	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count, sizet align) requires(!IsVoid<T>)
	{
		return static_cast<T*>(Alloc(arena, sizeof(T) * count, align));
	}
	template<typename T, Derived<Arena> ArenaT>
	bool Resize(ArenaT& arena, T* ptr, sizet ptrCount, sizet count) requires(!IsVoid<T>)
	{
		return Resize(arena, ptr, sizeof(T) * ptrCount, sizeof(T) * count);
	}
	template<typename T, Derived<Arena> ArenaT>
	void Free(ArenaT& arena, T* ptr, u32 count) requires(!IsVoid<T>)
	{
		Free(arena, static_cast<void*>(ptr), sizeof(T) * count);
	}
}    // namespace p
