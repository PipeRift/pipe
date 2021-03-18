// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Reflection/Registry/Registry.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "Reflection/Static/EnumType.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	template <typename T>
	struct TStaticEnumInitializer
	{
		static constexpr bool reflected = false;
		static const TFunction<EnumType*()> onInit;
	};
	template <typename T>
	inline const TFunction<EnumType*()> TStaticEnumInitializer<T>::onInit{};


	/**
	 * Enum Type Builder
	 * Builds enum types during static initialization
	 */
	template <typename T>
	struct TEnumTypeBuilder : public TypeBuilder
	{
	public:
		TEnumTypeBuilder() = default;
		TEnumTypeBuilder(Name name) : TypeBuilder(TypeId::Get<T>(), name) {}

		EnumType* GetType() const
		{
			return static_cast<EnumType*>(initializedType);
		}

	protected:
		Type* Build() override
		{
			EnumType& newType = ReflectionRegistry::Get().AddType<EnumType>(GetId());

			newType.id   = id;
			newType.name = name;
			return &newType;
		}
	};
}    // namespace Rift::Refl
