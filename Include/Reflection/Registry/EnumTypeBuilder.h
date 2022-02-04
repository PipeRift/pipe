// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Reflection/EnumType.h"
#include "Reflection/Registry/Registry.h"
#include "Reflection/Registry/StaticInitializers.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


#define ENUM(type)                                                    \
	template<>                                                        \
	struct Rift::Refl::TStaticEnumInitializer<type>                   \
	{                                                                 \
		static constexpr bool enabled = true;                         \
		static const Rift::TFunction<Rift::Refl::EnumType*()> onInit; \
	};                                                                \
	inline const Rift::TFunction<Rift::Refl::EnumType*()>             \
	    Rift::Refl::TStaticEnumInitializer<type>::onInit = []() {     \
		    Rift::Refl::TEnumTypeBuilder<type> builder{};             \
		    builder.Initialize();                                     \
		    return builder.GetType();                                 \
	    };


namespace Rift::Refl
{
	/**
	 * Enum Type Builder
	 * Builds enum types during static initialization
	 */
	template<typename T>
	struct TEnumTypeBuilder : public TypeBuilder
	{
	public:
		TEnumTypeBuilder() : TypeBuilder(TypeId::Get<T>(), GetTypeName<T>(false)) {}

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
