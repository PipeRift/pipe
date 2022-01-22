// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift::Memory
{
	class CORE_API Block
	{
	protected:
		void* data = nullptr;
		sizet size = 0;


	public:
		const void* GetData() const
		{
			return data;
		}

		void* GetData()
		{
			return data;
		}

		sizet GetSize() const
		{
			return size;
		}

		bool IsAllocated() const
		{
			return data != nullptr;
		}

		void* GetEnd() const
		{
			return static_cast<u8*>(data) + size;
		}

		bool Contains(void* ptr) const
		{
			return data <= ptr && static_cast<u8*>(data) + size > ptr;
		}

		const void* operator*() const
		{
			return GetData();
		}

		void* operator*()
		{
			return GetData();
		}
	};
}    // namespace Rift::Memory
