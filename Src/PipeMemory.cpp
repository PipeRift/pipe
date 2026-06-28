// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeMemory.h"

#include "PipeMemoryArenas.h"

#include <cstring>
#include <shared_mutex>

#if P_PLATFORM_WINDOWS
	#include <malloc.h>    // _aligned_malloc, _aligned_free, _aligned_realloc
#endif

namespace p
{
#pragma region Memory Ops
	void MoveMem(void* dest, void* src, sizet size)
	{
		memmove(dest, src, size);
	}
	void CopyMem(void* dest, const void* src, sizet size)
	{
		memcpy(dest, src, size);
	}
	void SwapMem(void* a, void* b, sizet size)
	{
		switch (size)
		{
			case 0: break;
			case 1: Swap(*static_cast<u8*>(a), *static_cast<u8*>(b)); break;
			case 2: Swap(*static_cast<u16*>(a), *static_cast<u16*>(b)); break;
			case 3:
				Swap(*reinterpret_cast<u16*&>(a)++, *reinterpret_cast<u16*&>(b)++);
				Swap(*static_cast<u8*>(a), *static_cast<u8*>(b));
				break;
			case 4: Swap(*static_cast<u32*>(a), *static_cast<u32*>(b)); break;
			case 5:
				Swap(*reinterpret_cast<u32*&>(a)++, *reinterpret_cast<u32*&>(b)++);
				Swap(*reinterpret_cast<u8*>(a), *reinterpret_cast<u8*>(b));
				break;
			case 6:
				Swap(*reinterpret_cast<u32*&>(a)++, *reinterpret_cast<u32*&>(b)++);
				Swap(*static_cast<u16*>(a), *static_cast<u16*>(b));
				break;
			case 7:
				Swap(*reinterpret_cast<u32*&>(a)++, *reinterpret_cast<u32*&>(b)++);
				Swap(*reinterpret_cast<u16*&>(a)++, *reinterpret_cast<u16*&>(b)++);
				Swap(*static_cast<u8*>(a), *static_cast<u8*>(b));
				break;
			case 8: Swap(*static_cast<u64*>(a), *static_cast<u64*>(b)); break;
			case 16:
				Swap(static_cast<u64*>(a)[0], static_cast<u64*>(b)[0]);
				Swap(static_cast<u64*>(a)[1], static_cast<u64*>(b)[1]);
				break;
			default: {    // Byte by byte swap. Optimized by the compiler
				u8* p          = static_cast<u8*>(a);
				u8* q          = static_cast<u8*>(b);
				u8* const pend = p + size;
				for (; p < pend; ++p, ++q)
				{
					Swap(*p, *q);
				}
			}
			break;
		}
	}
	void SetMem(void* dest, u8 value, sizet size)
	{
		memset(dest, value, size);
	}
	void SetZeroMem(void* dest, sizet size)
	{
		memset(dest, 0, size);
	}
	i32 CmpMem(const void* a, const void* b, sizet size)
	{
		return memcmp(a, b, size);
	}


	void* GetAlignedBlock(void* ptr, const sizet blockSize)
	{
		P_Check(IsPowerOfTwo(blockSize));
		const uPtr alignedPtr = uPtr(ptr) & ~(blockSize - 1);
		// return alignedPtr without casting to int (performance-no-int-to-ptr)
		return static_cast<u8*>(ptr) - (uPtr(ptr) - alignedPtr);
	}

	sizet GetAlignmentPadding(const void* ptr, sizet align)
	{
		P_Check(IsPowerOfTwo(align));
		return -reinterpret_cast<ssizet>(ptr) & (align - 1);
	}

	sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize)
	{
		// Get padding with the header as an offset
		return headerSize + GetAlignmentPadding(static_cast<const u8*>(ptr) + headerSize, align);
	}
#pragma endregion Memory Ops


#pragma region Allocation
	static Arena* currentArena = nullptr;
	static TArray<Arena*> arenaStack;

	void InitializeMemory()
	{
		PushCurrentArena(GetHeapArena());
	}

	void ShutdownMemory()
	{
		PopCurrentArena();
	}

	void* HeapAlloc(sizet size)
	{
#if P_PLATFORM_WINDOWS
		return _aligned_malloc(size, alignof(std::max_align_t));
#else
		return malloc(size);
#endif
	}

	void* HeapAlloc(sizet size, sizet align)
	{
#if P_PLATFORM_WINDOWS
		return _aligned_malloc(size, align);
#else
		return aligned_alloc(align, size);
#endif
	}

	void* HeapRealloc(void* ptr, sizet size)
	{
#if P_PLATFORM_WINDOWS
		return _aligned_realloc(ptr, size, alignof(std::max_align_t));
#else
		return realloc(ptr, size);
#endif
	}

	void HeapFree(void* ptr)
	{
#if P_PLATFORM_WINDOWS
		_aligned_free(ptr);
#else
		free(ptr);
#endif
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

	void PushCurrentArena(Arena& arena)
	{
		if (currentArena)
		{
			arenaStack.Add(currentArena);
		}
		currentArena = &arena;
	}

	void PopCurrentArena()
	{
		if (!arenaStack.IsEmpty())
		{
			currentArena = arenaStack.Last();
			arenaStack.RemoveLast();
		}
		else
		{
			currentArena = nullptr;
		}
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
#pragma endregion Allocation


#pragma region Arena
	// Dont use TArray to prevent initialization loop
	static std::vector<RegisteredArenaPtr> arenas;
	static std::vector<RegisteredArenaPtr> arenasSnapshot;
	static std::shared_mutex arenasMutex;


	ArenaBlock::ArenaBlock(ArenaBlock&& other) noexcept
	{
		data = Exchange(other.data, nullptr);
		size = Exchange(other.size, 0);
	}
	ArenaBlock& ArenaBlock::operator=(ArenaBlock&& other) noexcept
	{
		data = Exchange(other.data, nullptr);
		size = Exchange(other.size, 0);
		return *this;
	}


	Arena::Arena()
	{
		// Register arena
		std::unique_lock lock(arenasMutex);
		RegisteredArenaPtr entry;
		entry.arena  = this;
		entry.typeId = GetTypeId();
		arenas.push_back(entry);
	}
	Arena::~Arena()
	{
		std::unique_lock lock(arenasMutex);
		for (auto it = arenas.begin(); it != arenas.end(); ++it)
		{
			if (it->arena == this)
			{
				arenas.erase(it);
				break;
			}
		}
	}

	ChildArena::ChildArena(Arena* inParent) : parent{inParent}
	{
		if (!parent)
		{
			parent = &GetCurrentArena();
			if (parent == this) [[unlikely]]
			{
				parent = &GetHeapArena();
			}
		}
	}


	TView<const RegisteredArenaPtr> GetAllArenas()
	{
		std::shared_lock lock(arenasMutex);
		arenasSnapshot = arenas;
		return {arenasSnapshot.data(), static_cast<i32>(arenasSnapshot.size())};
	}

#pragma endregion Arena
}    // namespace p
