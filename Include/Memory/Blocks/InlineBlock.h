// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseBlock.h"


namespace Rift::Memory
{
	template <size_t Size>
	class CORE_API InlineBlock : BaseBlock
	{
	public:
		static constexpr size_t size = Size;

	protected:
		u8 data[size]{};


	public:
		InlineBlock()                   = default;
		~InlineBlock()                  = default;
		InlineBlock(const InlineBlock&) = default;
		InlineBlock(InlineBlock&&)      = delete;    // Can't move
		InlineBlock& operator=(const InlineBlock&) = default;
		InlineBlock& operator=(InlineBlock&&) = delete;    // Can't move

		// Allocate and Free don't do anything
		void Allocate(size_t) {}
		void Free() {}

		const void* GetData() const
		{
			return &data;
		}
		void* GetData()
		{
			return &data;
		}
		static constexpr size_t GetSize()
		{
			return size;
		}
	};
}    // namespace Rift::Memory
