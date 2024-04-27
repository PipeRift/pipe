// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Export.h"


namespace p::Memory
{
	struct PIPE_API Block
	{
		void* data = nullptr;
		sizet size = 0;


		Block() = default;
		Block(void* data, sizet size) : data{data}, size{size} {}
		Block(Block&& other) noexcept;
		Block& operator=(Block&& other) noexcept;
		Block(const Block& other)            = default;
		Block& operator=(const Block& other) = default;


		bool IsAllocated() const
		{
			return data != nullptr;
		}

		void* Begin() const
		{
			return data;
		}
		void* End() const
		{
			return static_cast<u8*>(data) + size;
		}

		bool Contains(void* ptr) const
		{
			return data <= ptr && static_cast<u8*>(data) + size > ptr;
		}

		const void* operator*() const
		{
			return data;
		}

		void* operator*()
		{
			return data;
		}
	};
}    // namespace p::Memory
