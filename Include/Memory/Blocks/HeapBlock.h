// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"
#include "Memory/Blocks/Block.h"


namespace pipe::Memory
{
	class CORE_API HeapBlock : public Block
	{
	public:
		HeapBlock() = default;
		HeapBlock(sizet initialSize);
		~HeapBlock();
		HeapBlock(const HeapBlock& other);
		HeapBlock(HeapBlock&& other);
		HeapBlock& operator=(const HeapBlock& other);
		HeapBlock& operator=(HeapBlock&& other);

		void Allocate(sizet size);
		void Free();
	};


	inline HeapBlock::HeapBlock(sizet initialSize)
	{
		Allocate(initialSize);
	}
	inline HeapBlock::~HeapBlock()
	{
		if (data)
		{
			Free();
		}
	}
	inline HeapBlock::HeapBlock(const HeapBlock& other)
	{
		if (other.data)
		{
			Allocate(other.size);
			memcpy(other.data, data, size);
		}
	}
	inline HeapBlock::HeapBlock(HeapBlock&& other)
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
	}
	inline HeapBlock& HeapBlock::operator=(const HeapBlock& other)
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
	inline HeapBlock& HeapBlock::operator=(HeapBlock&& other)
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
		return *this;
	}

	inline void HeapBlock::Allocate(sizet newSize)
	{
		size = newSize;
		data = pipe::Alloc(newSize);
	}
	inline void HeapBlock::Free()
	{
		pipe::Free(data);
		data = nullptr;
		size = 0;
	}
}    // namespace pipe::Memory
