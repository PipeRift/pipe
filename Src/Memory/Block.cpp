// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/Block.h"

#include "Pipe/Core/Checks.h"

#include <cstring>


namespace p::Memory
{
	Block::Block(Block&& other) noexcept
	{
		data = std::exchange(other.data, nullptr);
		size = std::exchange(other.size, 0);
	}
	Block& Block::operator=(Block&& other) noexcept
	{
		data = std::exchange(other.data, nullptr);
		size = std::exchange(other.size, 0);
		return *this;
	}
}    // namespace p::Memory
