// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"


namespace Rift::Memory
{
	class CORE_API MallocBlock
	{
	protected:
		void* data  = nullptr;
		size_t size = 0;


	public:
		MallocBlock() = default;
		MallocBlock(size_t initialSize);
		~MallocBlock();
		MallocBlock(const MallocBlock& other);
		MallocBlock(MallocBlock&& other);
		MallocBlock& operator=(const MallocBlock& other);
		MallocBlock& operator=(MallocBlock&& other);
		void* operator*() const
		{
			return data;
		}

		void Allocate(size_t size);
		void Free();

		const void* GetData() const;
		void* GetData();
		size_t GetSize() const;
	};


	inline MallocBlock::MallocBlock(size_t initialSize)
	{
		Allocate(initialSize);
	}
	inline MallocBlock::~MallocBlock()
	{
		if (data)
		{
			Free();
		}
	}
	inline MallocBlock::MallocBlock(const MallocBlock& other)
	{
		if (other.data)
		{
			Allocate(other.size);
			memcpy(other.data, data, size);
		}
	}
	inline MallocBlock::MallocBlock(MallocBlock&& other)
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
	}
	inline MallocBlock& MallocBlock::operator=(const MallocBlock& other)
	{
		if (!other.data)
		{
			Free();
			return;
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
	}
	inline MallocBlock& MallocBlock::operator=(MallocBlock&& other)
	{
		data       = other.data;
		size       = other.size;
		other.data = nullptr;
		other.size = 0;
		return *this;
	}

	inline void MallocBlock::Allocate(size_t newSize)
	{
		size = newSize;
		data = Rift::Alloc(newSize);
	}
	inline void MallocBlock::Free()
	{
		Rift::Free(data);
		data = nullptr;
		size = 0;
	}

	inline const void* MallocBlock::GetData() const
	{
		return data;
	}
	inline void* MallocBlock::GetData()
	{
		return data;
	}
	inline size_t MallocBlock::GetSize() const
	{
		return size;
	}
}    // namespace Rift::Memory
