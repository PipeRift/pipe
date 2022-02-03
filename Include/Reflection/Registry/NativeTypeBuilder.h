// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/NativeType.h"
#include "Reflection/Registry/Registry.h"
#include "Reflection/Registry/StaticInitializers.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"



#define REFLECT_NATIVE_TYPE(type)                                                \
	template<>                                                                   \
	struct Rift::Refl::TStaticNativeInitializer<type>                            \
	{                                                                            \
		static constexpr bool enabled = true;                                    \
		static const Rift::TFunction<Rift::Refl::NativeType*()> onInit;          \
	};                                                                           \
	inline const Rift::TFunction<Rift::Refl::NativeType*()>                      \
	    Rift::Refl::TStaticNativeInitializer<type>::onInit = []() {              \
		    Rift::Refl::TNativeTypeBuilder<type> builder{Rift::Name{TX(#type)}}; \
		    builder.Initialize();                                                \
		    return builder.GetType();                                            \
	    };


namespace Rift::Refl
{
	/**
	 * Native Type Builder
	 * Builds native types during static initialization
	 */
	template<typename T>
	struct TNativeTypeBuilder : public TypeBuilder
	{
	public:
		TNativeTypeBuilder() = default;
		TNativeTypeBuilder(Name name) : TypeBuilder(TypeId::Get<T>(), name) {}

		NativeType* GetType() const
		{
			return static_cast<NativeType*>(initializedType);
		}

	protected:
		Type* Build() override
		{
			NativeType& newType = ReflectionRegistry::Get().AddType<NativeType>(GetId());

			newType.id   = id;
			newType.name = name;
			return &newType;
		}
	};
}    // namespace Rift::Refl
