// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Blocks/Block.h"


namespace pipe::Memory
{
	template<sizet Size>
	class CORE_API InlineBlock : BaseBlock
	{
	private:
		u8 inlineData[Size];


	public:
		InlineBlock()
		{
			data = &inlineData;
			size = Size;
		}
		~InlineBlock() = default;

		template<sizet Size2>
		InlineBlock(const InlineBlock<Size2>& other) : InlineBlock() requires(Size >= Size2)
		{
			inlineData = other.inlineData;
		}
		template<sizet Size2>
		InlineBlock& operator=(const InlineBlock&) requires(Size >= Size2)
		{
			inlineData = other.inlineData;
		}
		InlineBlock(InlineBlock&&) = delete;               // Can't move
		InlineBlock& operator=(InlineBlock&&) = delete;    // Can't move

		// Allocate and Free don't do anything
		void Allocate(sizet) {}
		void Free() {}
	};
}    // namespace pipe::Memory
