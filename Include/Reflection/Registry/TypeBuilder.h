// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	struct TypeId;


	struct CORE_API TypeBuilder
	{
	protected:
		TypeId id;
		Name name;
		Type* initializedType = nullptr;


	public:
		TypeBuilder() = default;
		TypeBuilder(TypeId id, Name name);
		virtual ~TypeBuilder() {}

		void Initialize();

		TypeId GetId() const;
		Name GetName() const;

	protected:
		virtual Type* Build() = 0;
	};
}    // namespace Rift::Refl
