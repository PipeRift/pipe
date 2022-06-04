// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Pipe::Memory
{
	/** This is an Arena interface.
	 * It serves the single purpose of documenting an Arena's interface.
	 * Not intended to be used.
	 */
	class CORE_API DummyArena
	{
	public:
		DummyArena() {}
		~DummyArena() {}

		// Define copy or move as desired
		DummyArena(const DummyArena&) = default;
		DummyArena(DummyArena&&)      = default;
		DummyArena& operator=(const DummyArena&) = default;
		DummyArena& operator=(DummyArena&&) = default;

		void* Allocate(const sizet size)
		{
			return nullptr;
		}
		void* Allocate(const sizet size, const sizet alignment)
		{
			return nullptr;
		}
		void Free(void* ptr) {}
	};
}    // namespace Pipe::Memory
