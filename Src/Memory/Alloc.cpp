// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/Alloc.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Profiler.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/BigBestFitArena.h"
#include "Pipe/Memory/HeapArena.h"

#include <cstdlib>
#include <memory>


namespace p
{
	static Arena* currentArena = nullptr;


	void* HeapAlloc(sizet size)
	{
		void* const ptr = std::malloc(size);
		// FIX: Profiler reports alloc gets called frequently twice with the same pointer. Seems
		// related to allocators
		// TracyAllocS(p, n, 12);
		return ptr;
	}

	void* HeapAlloc(sizet size, sizet align)
	{
#if PLATFORM_WINDOWS
		// TODO: Windows needs _aligned_free in order to use _aligned_alloc()
		void* const ptr = std::malloc(size);
#elif PLATFORM_MACOS || PLATFORM_LINUX
		void* p;
		(void)(posix_memalign(&p, align, n));
#else
		void* const p = std::aligned_alloc(align, n);
#endif
		// TracyAllocS(p, n, 8);
		return ptr;
	}

	void* HeapRealloc(void* ptr, sizet size)
	{
		// TracyFreeS(old, 8);
		void* const p = std::realloc(ptr, size);
		// TracyAllocS(p, size, 8);
		return p;
	}

	void HeapFree(void* ptr)
	{
		// TracyFreeS(p, 8);
		std::free(ptr);
	}


	HeapArena& GetHeapArena()
	{
		static HeapArena heapArena{};
		return heapArena;
	}

	Arena* GetCurrentArena()
	{
		return currentArena ? currentArena : &GetHeapArena();
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
		return Alloc(*GetCurrentArena(), size);
	}

	void* Alloc(sizet size, sizet align)
	{
		return Alloc(*GetCurrentArena(), size, align);
	}

	bool Resize(void* ptr, sizet ptrSize, sizet size)
	{
		return Resize(*GetCurrentArena(), ptr, ptrSize, size);
	}

	void Free(void* ptr, sizet size)
	{
		Free(*GetCurrentArena(), ptr, size);
	}
}    // namespace p
