// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

namespace p
{
	/** This is an Arena interface.
	 * It serves the single purpose of documenting an Arena's interface.
	 * Not intended to be used.
	 */
	class PIPE_API DummyArena
	{
	public:
		~DummyArena() {}

		// Define copy or move as desired
		DummyArena(const DummyArena&)            = default;
		DummyArena(DummyArena&&)                 = default;
		DummyArena& operator=(const DummyArena&) = default;
		DummyArena& operator=(DummyArena&&)      = default;

		void* Alloc(const sizet size)
		{
			return nullptr;
		}
		void* Alloc(const sizet size, const sizet alignment)
		{
			return nullptr;
		}
		bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr) {}
	};
}    // namespace p
