// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Memory/Alloc.h"

#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/HeapArena.h"
#include "Pipe/Memory/MemoryStats.h"

#include <cstdlib>
#include <memory>


namespace p
{
	static Arena* currentArena = nullptr;

	void InitializeMemory()
	{
		GetHeapArena();
	}

	void* HeapAlloc(sizet size)
	{
		return malloc(size);
	}

	void* HeapAlloc(sizet size, sizet align)
	{
#if P_PLATFORM_WINDOWS
		// TODO: Windows needs _aligned_free in order to use _aligned_alloc()
		void* const ptr = malloc(size);
#else
		void* const ptr = aligned_alloc(align, size);
#endif
		return ptr;
	}

	void* HeapRealloc(void* ptr, sizet size)
	{
		return realloc(ptr, size);
	}

	void HeapFree(void* ptr)
	{
		free(ptr);
	}


	HeapArena& GetHeapArena()
	{
		static HeapArena heapArena{};
		return heapArena;
	}

	Arena& GetCurrentArena()
	{
		return currentArena ? *currentArena : GetHeapArena();
	}

	void SetCurrentArena(Arena& arena)
	{
		currentArena = &arena;
	}


	void* Alloc(Arena& arena, sizet size)
	{
		return arena.Alloc(size);
	}

	void* Alloc(Arena& arena, sizet size, sizet align)
	{
		return arena.Alloc(size, align);
	}

	bool Realloc(Arena& arena, void* ptr, sizet ptrSize, sizet size)
	{
		return arena.Realloc(ptr, ptrSize, size);
	}

	void Free(Arena& arena, void* ptr, sizet size)
	{
		arena.Free(ptr, size);
	}

	void* Alloc(sizet size)
	{
		return Alloc(GetCurrentArena(), size);
	}

	void* Alloc(sizet size, sizet align)
	{
		return Alloc(GetCurrentArena(), size, align);
	}

	bool Realloc(void* ptr, sizet ptrSize, sizet size)
	{
		return Realloc(GetCurrentArena(), ptr, ptrSize, size);
	}

	void Free(void* ptr, sizet size)
	{
		Free(GetCurrentArena(), ptr, size);
	}
}    // namespace p
