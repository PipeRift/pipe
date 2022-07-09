// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/delegate.hpp"
#include "Pipe/Core/Function.h"
#include "Pipe/Memory/Alloc.h"


namespace p
{
	/** Arena defines the API used on all other arena types */
	class PIPE_API Arena
	{
	public:
		virtual ~Arena() {}

		virtual void* Alloc(sizet size)              = 0;
		virtual void* Alloc(sizet size, sizet align) = 0;
		virtual bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		virtual void Free(void* ptr, sizet size) = 0;
	};

	class PIPE_API ChildArena : public Arena
	{
	protected:
		Arena* parent = nullptr;

	public:
		ChildArena(Arena* parent);

		Arena& GetParentArena() const
		{
			return *parent;
		}
	};
}    // namespace p
