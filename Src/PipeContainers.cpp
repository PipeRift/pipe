// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeContainers.h"


namespace p
{
	BitArray::BitArray(i32 newSize) : size(newSize), bits(CalculateDataSize(newSize)) {}
	BitArray::BitArray(i32 newSize, bool value)
	    : size(newSize), bits(((size - 1) >> 5) + 1, value ? 0xffffffff : 0x00000000)
	{}
	BitArray::BitArray(const u32* data, i32 newSize)
	    : size{newSize}, bits(data, CalculateDataSize(newSize))
	{}

	BitArray::BitArray(const bool* data, i32 newSize)
	    : size{newSize}, bits(CalculateDataSize(newSize))
	{
		for (i32 i = 0; i < newSize; ++i)
		{
			if (data[i])
			{
				SetTrue(i);
			}
		}
	}

	BitArray::BitArray(Arena& arena, i32 newSize)
	    : size(newSize), bits(arena, CalculateDataSize(newSize))
	{}
	BitArray::BitArray(Arena& arena, i32 newSize, bool value)
	    : size(newSize), bits(arena, CalculateDataSize(newSize), value ? 0xffffffff : 0x00000000)
	{}
	BitArray::BitArray(Arena& arena, const u32* data, i32 newSize)
	    : size{newSize}, bits(arena, data, CalculateDataSize(newSize))
	{}

	BitArray::BitArray(Arena& arena, const bool* data, i32 newSize)
	    : size{newSize}, bits(arena, CalculateDataSize(newSize))
	{
		for (i32 i = 0; i < newSize; ++i)
		{
			if (data[i])
			{
				SetTrue(i);
			}
		}
	}

	BitArray::BitArray(BitArray&& other) noexcept
	{
		size       = other.size;
		other.size = 0;
		bits       = Move(other.bits);
	}
	BitArray::BitArray(const BitArray& other)
	{
		size = other.size;
		bits = other.bits;
	}
	BitArray& BitArray::operator=(BitArray&& other) noexcept
	{
		size       = other.size;
		other.size = 0;
		bits       = Move(other.bits);
		return *this;
	}
	BitArray& BitArray::operator=(const BitArray& other)
	{
		size = other.size;
		bits = other.bits;
		return *this;
	}

	/** @return true if a bit is set */
	bool BitArray::IsSet(i32 index) const
	{
		return bits[(index >> 5)] >> (index & 0x0000001f) & 0x00000001;
	}

	void BitArray::SetAllTrue()
	{
		FillBitArray(0xffffffff);
	}

	void BitArray::SetAllFalse()
	{
		FillBitArray(0x00000000);
	}

	void BitArray::SetAll(bool value)
	{
		FillBitArray(value ? 0xffffffff : 0x00000000);
	}

	void BitArray::SetTrue(i32 index)
	{
		const u32 bitOffset = index & 0x0000001f;    // index % 32
		bits[index >> 5] |= 0x00000001 << bitOffset;
	}

	void BitArray::SetFalse(i32 index)
	{
		const i32 bitOffset = index & 0x0000001f;    // index % 32
		bits[index >> 5] &= ~(0x00000001 << bitOffset);
	}

	void BitArray::Set(i32 index, bool value)
	{
		const i32 bitOffset = index & 0x0000001f;    // index % 32
		u32& word           = bits[index >> 5];
		word                = (word & ~(1 << bitOffset)) | (u32(value) << bitOffset);
	}

	void BitArray::FillBitArray(u32 pattern)
	{
		for (i32 i = 0; i < bits.Size(); ++i)
		{
			bits[i] = pattern;
		}
	}

	void BitArray::Resize(i32 newSize, Shrink shouldShrink)
	{
		size = newSize;
		bits.Resize(((newSize - 1) >> 5) + 1, shouldShrink);
	}

	void BitArray::Resize(i32 newSize, bool value, Shrink shouldShrink)
	{
		size = newSize;
		bits.Resize(((newSize - 1) >> 5) + 1, value ? 0xffffffff : 0x00000000, shouldShrink);
	}

	void BitArray::Clear()
	{
		size = 0;
		bits.Clear();
	}

	i32 BitArray::GetNextSet(i32 index) const
	{
		i32 i;
		for (i = index + 1; i < size; ++i)
		{
			if (IsSet(i))
			{
				return i;
			}
		}
		for (i = 0; i < index - 1; ++i)
		{
			if (IsSet(i))
			{
				return i;
			}
		}
		return NO_INDEX;
	}

	i32 BitArray::GetPreviousSet(i32 index) const
	{
		i32 i;
		if (index != 0)
		{
			for (i = index - 1; i > 0; --i)
			{
				if (IsSet(i))
				{
					return i;
				}
			}
			if (IsSet(0))
			{
				return 0;
			}
		}

		for (i = size - 1; i > index; --i)
		{
			if (IsSet(i))
			{
				return i;
			}
		}
		return NO_INDEX;
	}

	i32 BitArray::CountSetBits(i32 fromIndex, i32 toIndex) const
	{
		if (toIndex == NO_INDEX)
		{
			toIndex = size;
		}

		P_Check(fromIndex >= 0);
		P_Check(toIndex >= fromIndex && toIndex <= size);

		i32 numSetBits = 0;
		// To data indices
		fromIndex = fromIndex >> 5;
		toIndex   = toIndex >> 5;
		for (i32 i = fromIndex; i < toIndex; ++i)
		{
			numSetBits += CountBits(bits[i]);
		}
		return numSetBits;
	}
}    // namespace p
