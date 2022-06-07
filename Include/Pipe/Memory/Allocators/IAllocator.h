// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

namespace p::Memory
{
	class PIPE_API IAllocator
	{
	public:
		IAllocator() = default;
	};

	template<typename T>
	class TDummyAllocator : public IAllocator
	{
	public:
		T* Allocate(const sizet count)
		{
			return nullptr;
		}
		T* Allocate(const sizet count, const sizet align)
		{
			return nullptr;
		}

		void Free(T* ptr) {}
	};
}    // namespace p::Memory
