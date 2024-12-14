// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "PipeArrays.h"
#include "PipePlatform.h"

#include <cstdlib>
#include <shared_mutex>


namespace p
{
	struct PIPE_API AllocationStats
	{
		u8* ptr  = nullptr;
		u64 size = 0;
	};

	struct PIPE_API SortLessAllocationStats
	{
		bool operator()(const AllocationStats& a, const AllocationStats& b) const
		{
			return a.ptr + a.size < b.ptr;
		}

		bool operator()(void* a, const AllocationStats& b) const
		{
			return a < b.ptr;
		}

		bool operator()(const AllocationStats& a, void* b) const
		{
			return a.ptr + a.size < b;
		}
	};

	struct PIPE_API MemoryStats
	{
		const char* name = "Arena";

		sizet used      = 0;
		sizet available = 0;
		mutable std::shared_mutex mutex;
		TArray<AllocationStats> allocations;
		TArray<AllocationStats> freedAllocations;


		MemoryStats();
		~MemoryStats();

		void Add(void* ptr, sizet size);
		void Remove(void* ptr, sizet size);
		void Release();

	private:
		void PrintAllocationError(StringView error, AllocationStats* allocation);
	};
}    // namespace p
