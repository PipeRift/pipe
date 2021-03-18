// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	struct CORE_API TypeBuilder
	{
	protected:
		TypeId id;
		Name name;
		Type* initializedType = nullptr;


	public:
		TypeBuilder() = default;
		TypeBuilder(TypeId id, Name name) : id{id}, name{name} {}
		virtual ~TypeBuilder() {}

		void Initialize();

		TypeId GetId() const
		{
			return id;
		}
		Name GetName() const
		{
			return name;
		}

	protected:
		virtual Type* Build() = 0;
	};
}