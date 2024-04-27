// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Type.h"


namespace p
{
	struct TypeId;


	struct PIPE_API TypeBuilder
	{
	protected:
		TypeId id;
		StringView name;
		Type* initializedType = nullptr;


	public:
		TypeBuilder() = default;
		TypeBuilder(TypeId id, StringView name);
		virtual ~TypeBuilder() {}

		bool BeginBuild();
		void EndBuild();

		TypeId GetId() const;
		StringView GetName() const;

		Type* GetType() const
		{
			return initializedType;
		}

	protected:
		virtual Type* CreateType() = 0;
	};
}    // namespace p
