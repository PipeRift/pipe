// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseBlock.h"


namespace Rift::Memory
{
	template <sizet Size>
	class CORE_API InlineBlock : BaseBlock
	{
	public:
		static constexpr sizet size = Size;

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
		void Allocate(sizet) {}
		void Free() {}

		const void* GetData() const
		{
			return &data;
		}
		void* GetData()
		{
			return &data;
		}
		static constexpr sizet GetSize()
		{
			return size;
		}

		bool IsAllocated() const
		{
			return true;
		}
	};
}    // namespace Rift::Memory
