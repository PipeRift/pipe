// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/Blocks/HeapBlock.h"

#include <cstring>


namespace p::Memory
{
	HeapBlock::HeapBlock(sizet initialSize)
	{
		Alloc(initialSize);
	}

	HeapBlock::~HeapBlock()
	{
		if (data)
		{
			Free();
		}
	}

	HeapBlock::HeapBlock(const HeapBlock& other) : Block(other)
	{
		if (other.data)
		{
			Alloc(other.size);
			std::memcpy(other.data, data, size);
		}
	}

	HeapBlock::HeapBlock(HeapBlock&& other) noexcept
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
	}

	HeapBlock& HeapBlock::operator=(const HeapBlock& other)
	{
		if (!other.data)
		{
			Free();
			return *this;
		}
		else if (size != other.size)
		{
			if (size > 0)
			{
				Free();
			}
			Alloc(other.size);
		}
		std::memcpy(other.data, data, size);
		return *this;
	}

	HeapBlock& HeapBlock::operator=(HeapBlock&& other) noexcept
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
		return *this;
	}

	void HeapBlock::Alloc(sizet newSize)
	{
		size = newSize;
		data = p::Alloc(newSize);
	}

	void HeapBlock::Free()
	{
		p::Free(data);
		data = nullptr;
		size = 0;
	}
}    // namespace p::Memory
