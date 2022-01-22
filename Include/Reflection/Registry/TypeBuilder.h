// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	struct TypeId;


	struct TypeBuilder
	{
	protected:
		TypeId id;
		Name name;
		Type* initializedType = nullptr;


	public:
		CORE_API TypeBuilder() = default;
		CORE_API TypeBuilder(TypeId id, Name name);
		CORE_API virtual ~TypeBuilder() {}

		CORE_API void Initialize();

		CORE_API TypeId GetId() const;
		CORE_API Name GetName() const;

	protected:
		CORE_API virtual Type* Build() = 0;
	};
}    // namespace Rift::Refl
