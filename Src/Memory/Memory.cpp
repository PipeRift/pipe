// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Memory/Memory.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Utility.h"
#include "PipeMath.h"

#include <cstring>


namespace p
{
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
}    // namespace p
