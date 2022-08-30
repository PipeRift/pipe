// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Type.h"


namespace p
{
	struct TypeId;


	struct TypeBuilder
	{
	protected:
		TypeId id;
		StringView name;
		Type* initializedType = nullptr;


	public:
		PIPE_API TypeBuilder() = default;
		PIPE_API TypeBuilder(TypeId id, StringView name);
		PIPE_API virtual ~TypeBuilder() {}

		PIPE_API void Initialize();

		PIPE_API TypeId GetId() const;
		PIPE_API StringView GetName() const;

	protected:
		PIPE_API virtual Type* Build() = 0;
	};
}    // namespace p
