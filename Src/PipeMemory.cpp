// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeMemory.h"

#include "PipeMemoryArenas.h"

#include <cstring>

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
#pragma endregion Allocation


#pragma region Arena
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
#pragma endregion Arena
}    // namespace p
