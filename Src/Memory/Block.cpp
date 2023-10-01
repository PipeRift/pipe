// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Memory/Block.h"

#include "Pipe/Core/Utility.h"


namespace p::Memory
{
	Block::Block(Block&& other) noexcept
	{
		data = Exchange(other.data, nullptr);
		size = Exchange(other.size, 0);
	}
	Block& Block::operator=(Block&& other) noexcept
	{
		data = Exchange(other.data, nullptr);
		size = Exchange(other.size, 0);
		return *this;
	}
}    // namespace p::Memory
