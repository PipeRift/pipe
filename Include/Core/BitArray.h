// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Array.h"
#include "Core/Platform.h"


namespace pipe::core
{
	template<typename Allocator = Memory::TDefaultAllocator<u32>>
	class TBitArray
	{
		using ArrayType = TArray<u32, Allocator>;

	private:
		/** The number of bits in this array */
		i32 numBits;
		/** The array of u32s containing the bits */
		ArrayType bits;


	public:
		/** Constructors */
		TBitArray() : numBits(0) {}
		TBitArray(i32 newSize) : numBits(newSize), bits{((numBits - 1) >> 5) + 1} {}
		TBitArray(TBitArray&& other) noexcept
		{
			numBits       = other.numBits;
			other.numBits = 0;
			bits          = Move(other.bits);
		}
		TBitArray(const TBitArray& other)
		{
			numBits = other.numBits;
			bits    = other.bits;
		}

		TBitArray(u32 newSize, bool value)
		    : numBits(newSize), bits{((numBits - 1) >> 5) + 1, value ? 0xffffffff : 0x00000000}
		{}
		TBitArray(u32 newSize, u32* data) : numBits{newSize}, bits{data, ((numBits - 1) >> 5) + 1}
		{}

		TBitArray& operator=(TBitArray&& other) noexcept
		{
			numBits       = other.numBits;
			other.numBits = 0;
			bits          = Move(other.bits);
			return *this;
		}
		TBitArray& operator=(const TBitArray& other)
		{
			numBits = other.numBits;
			bits    = other.bits;
			return *this;
		}

		/** @return true if a bit is set */
		bool IsSet(u32 index) const
		{
			return bits[(index >> 5)] >> (index & 0x0000001f) & 0x00000001;
		}

		// Set all bits in this array
		void FillBits()
		{
			FillBitArray(0xffffffff);
		}

		// Clear all bits in this array
		void ClearBits()
		{
			FillBitArray(0x00000000);
		}
		// Set all bits in this array to value
		void SetBits(bool value)
		{
			FillBitArray(value ? 0xffffffff : 0x00000000);
		}

		// Set a single bit
		void FillBit(u32 index)
		{
			const u32 bitOffset = index & 0x0000001f;    // index % 32
			bits[index >> 5] |= 0x00000001 << bitOffset;
		}

		// Clear a single bit
		void ClearBit(u32 index)
		{
			const u32 bitOffset = index & 0x0000001f;    // index % 32
			bits[index >> 5] &= ~(0x00000001 << bitOffset);
		}

		void SetBit(u32 index, bool value)
		{
			const u32 bitOffset = index & 0x0000001f;    // index % 32
			u32& word           = bits[index >> 5];
			word                = (word & ~(1 << bitOffset)) | (u32(value) << bitOffset);
		}

		// Fill with a 32-bit pattern
		void FillBitArray(u32 pattern);

		// flip a single bit
		void FlipBit(u32 index)
		{
			SetBit(index, !IsSet(index));
		};

		void Resize(i32 size)
		{
			numBits = size;
			bits.Resize(((numBits - 1) >> 5) + 1);
		}

		void Clear()
		{
			numBits = 0;
			bits.Empty();
		}

		// Returns index of next set bit in array (wraps around)
		i32 GetNextSet(i32 index) const;

		// @return index of previous set bit in array (wraps around)
		i32 GetPreviousSet(i32 index) const;

		// @return the number of bits in this bit array
		i32 Size() const
		{
			return numBits;
		}
		// @return the number of bits reserved on the buffer
		i32 MaxSize() const
		{
			return bits.MaxSize() * sizeof(i32) * 8;
		}

		/** @return true if index is contained on the bit array */
		bool IsValidIndex(u32 index) const
		{
			return index < u32(Size());
		}

		/** @return true if a bit is set */
		bool IsEmpty() const
		{
			return Size() <= 0;
		}

		TBitArray operator~()
		{
			TBitArray result(numBits);
			for (u32 i = 0; i < bits.Size(); ++i)
			{
				result.bits[i] = ~bits[i];
			}
			return result;
		}
		TBitArray& operator^=(const TBitArray& other)
		{
			const i32 minSize = bits.Size() < other.bits.Size() ? bits.Size() : other.bits.Size();
			for (u32 i = 0; i < minSize; ++i)
			{
				bits[i] ^= other.bits[i];
			}
			return *this;
		}

		TBitArray& operator&=(const TBitArray& other)
		{
			const i32 minSize = bits.Size() < other.bits.Size() ? bits.Size() : other.bits.Size();
			for (u32 i = 0; i < minSize; ++i)
			{
				bits[i] &= other.bits[i];
			}
			return *this;
		}
		TBitArray& operator|=(const TBitArray& other)
		{
			const i32 minSize = bits.Size() < other.bits.Size() ? bits.Size() : other.bits.Size();
			for (i32 i = 0; i < minSize; ++i)
			{
				bits[i] |= other.bits[i];
			}
			return *this;
		}
		TBitArray operator^(const TBitArray& other)
		{
			TBitArray result((numBits < other.numBits) ? numBits : other.numBits);
			for (u32 i = 0; i < result.bits.Size(); ++i)
			{
				result.bits[i] = bits[i] ^ other.bits[i];
			}
			return result;
		}
		TBitArray operator&(const TBitArray& other)
		{
			TBitArray result((numBits < other.numBits) ? numBits : other.numBits);
			for (u32 i = 0; i < result.bits.Size(); ++i)
			{
				result.bits[i] = bits[i] & other.bits[i];
			}
			return result;
		}
		TBitArray operator|(const TBitArray& other)
		{
			TBitArray result((numBits < other.numBits) ? numBits : other.numBits);
			for (u32 i = 0; i < result.bits.Size(); ++i)
			{
				result.bits[i] = bits[i] | other.bits[i];
			}
			return result;
		}
	};


	template<typename Allocator>
	void TBitArray<Allocator>::FillBitArray(u32 pattern)
	{
		for (u32 i = 0; i < bits.Size(); ++i)
		{
			bits[i] = pattern;
		}
	}

	template<typename Allocator>
	i32 TBitArray<Allocator>::GetNextSet(i32 index) const
	{
		i32 i;
		for (i = index + 1; i < numBits; ++i)
		{
			if (IsSet(i))
				return i;
		}

		for (i = 0; i < index - 1; ++i)
		{
			if (IsSet(i))
				return i;
		}
		return -1;
	}

	template<typename Allocator>
	i32 TBitArray<Allocator>::GetPreviousSet(i32 index) const
	{
		i32 i;
		if (index != 0)
		{
			for (i = index - 1; i > 0; --i)
			{
				if (IsSet(i))
					return i;
			}
			if (IsSet(0))
				return 0;
		}

		for (i = numBits - 1; i > index; i--)
		{
			if (IsSet(i))
				return i;
		}
		return -1;
	}


	using BitArray = TBitArray<>;
}    // namespace pipe::core

namespace pipe
{
	using namespace pipe::core;
}