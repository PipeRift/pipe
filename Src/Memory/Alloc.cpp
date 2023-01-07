// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Memory/Alloc.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Profiler.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/BigBestFitArena.h"
#include "Pipe/Memory/HeapArena.h"
#include "Pipe/Memory/MemoryStats.h"

#include <cstdlib>
#include <memory>


namespace p
{
	static Arena* currentArena = nullptr;


	void* HeapAlloc(sizet size)
	{
		void* const ptr = std::malloc(size);
#if P_DEBUG
		// FIX: Profiler reports alloc gets called frequently twice with the same pointer. Seems
		// related to allocators
		// TracyAllocS(ptr, size, 12);
		// GetHeapStats()->Add(ptr, size);
#endif
		return ptr;
	}

	void* HeapAlloc(sizet size, sizet align)
	{
#if P_PLATFORM_WINDOWS
		// TODO: Windows needs _aligned_free in order to use _aligned_alloc()
		void* const ptr = std::malloc(size);
#else
		void* const ptr = std::aligned_alloc(align, size);
#endif
#if P_DEBUG
		// GetHeapStats()->Add(ptr, size);
#endif
		return ptr;
	}

	void* HeapRealloc(void* ptr, sizet size)
	{
#if P_DEBUG
		// GetHeapStats()->Remove(ptr);
#endif
		ptr = std::realloc(ptr, size);
#if P_DEBUG
		// GetHeapStats()->Add(ptr, size);
#endif
		return ptr;
	}

	void HeapFree(void* ptr)
	{
#if P_DEBUG
		// GetHeapStats()->Remove(ptr);
#endif
		std::free(ptr);
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

	MemoryStats* GetHeapStats()
	{
		static MemoryStats heapStats;
		return &heapStats;
	}


	void* Alloc(Arena& arena, sizet size)
	{
		return arena.Alloc(size);
	}

	void* Alloc(Arena& arena, sizet size, sizet align)
	{
		return arena.Alloc(size, align);
	}

	bool Resize(Arena& arena, void* ptr, sizet ptrSize, sizet size)
	{
		return arena.Resize(ptr, ptrSize, size);
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

	bool Resize(void* ptr, sizet ptrSize, sizet size)
	{
		return Resize(GetCurrentArena(), ptr, ptrSize, size);
	}

	void Free(void* ptr, sizet size)
	{
		Free(GetCurrentArena(), ptr, size);
	}
}    // namespace p
