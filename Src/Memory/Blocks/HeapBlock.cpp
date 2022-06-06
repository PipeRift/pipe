// Copyright 2015-2022 Piperift - All rights reserved

#include "Memory/Blocks/HeapBlock.h"

#include <memory>


namespace p::Memory
{
	HeapBlock::HeapBlock(sizet initialSize)
	{
		Allocate(initialSize);
	}

	HeapBlock::~HeapBlock()
	{
		if (data)
		{
			Free();
		}
	}

	HeapBlock::HeapBlock(const HeapBlock& other)
	{
		if (other.data)
		{
			Allocate(other.size);
			memcpy(other.data, data, size);
		}
	}

	HeapBlock::HeapBlock(HeapBlock&& other)
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
			Allocate(other.size);
		}
		memcpy(other.data, data, size);
		return *this;
	}

	HeapBlock& HeapBlock::operator=(HeapBlock&& other)
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
		return *this;
	}

	void HeapBlock::Allocate(sizet newSize)
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
