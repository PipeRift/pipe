
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Broadcast.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/ECS/SparseSet.h"


namespace p::ecs
{
	template<typename Allocator = HeapAllocator>
	struct TPoolSet : public BasicSparseSet<Allocator>
	{
		using Super = BasicSparseSet<Allocator>;

	public:
		using Super::Super;

		void PopSwap(Id id)
		{
			Super::SwapAndPop(id, nullptr);
		}

		void Pop(Id id)
		{
			Super::InPlacePop(id, nullptr);
		}
	};
}    // namespace p::ecs
