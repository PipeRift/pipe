// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"


namespace Rift::Memory
{
	class CORE_API HeapBlock
	{
	protected:
		void* data  = nullptr;
		size_t size = 0;


	public:
		HeapBlock() = default;
		HeapBlock(size_t initialSize);
		~HeapBlock();
		HeapBlock(const HeapBlock& other);
		HeapBlock(HeapBlock&& other);
		HeapBlock& operator=(const HeapBlock& other);
		HeapBlock& operator=(HeapBlock&& other);
		void* operator*() const
		{
			return data;
		}

		void Allocate(size_t size);
		void Free();

		const void* GetData() const;
		void* GetData();
		size_t GetSize() const;

		bool IsAllocated() const
		{
			return !!data;
		}
	};


	inline HeapBlock::HeapBlock(size_t initialSize)
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

	inline void HeapBlock::Allocate(size_t newSize)
	{
		size = newSize;
		data = Rift::Alloc(newSize);
	}
	inline void HeapBlock::Free()
	{
		Rift::Free(data);
		data = nullptr;
		size = 0;
	}

	inline const void* HeapBlock::GetData() const
	{
		return data;
	}
	inline void* HeapBlock::GetData()
	{
		return data;
	}
	inline size_t HeapBlock::GetSize() const
	{
		return size;
	}
}    // namespace Rift::Memory
