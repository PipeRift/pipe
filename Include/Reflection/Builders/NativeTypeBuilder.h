// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Builders/StaticInitializers.h"
#include "Reflection/Builders/TypeBuilder.h"
#include "Reflection/NativeType.h"
#include "Reflection/Registry.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"


#define REFLECT_NATIVE_TYPE(type)                                       \
	template<>                                                          \
	struct Pipe::Refl::TStaticNativeInitializer<type>                   \
	{                                                                   \
		static constexpr bool enabled = true;                           \
		static const Pipe::TFunction<Pipe::Refl::NativeType*()> onInit; \
	};                                                                  \
	inline const Pipe::TFunction<Pipe::Refl::NativeType*()>             \
	    Pipe::Refl::TStaticNativeInitializer<type>::onInit = []() {     \
		    Pipe::Refl::TNativeTypeBuilder<type> builder{};             \
		    builder.Initialize();                                       \
		    return builder.GetType();                                   \
	    };


namespace Pipe::Refl
{
	/**
	 * Native Type Builder
	 * Builds native types during static initialization
	 */
	template<typename T>
	struct TNativeTypeBuilder : public TypeBuilder
	{
	public:
		TNativeTypeBuilder() : TypeBuilder(GetTypeId<T>(), GetTypeName<T>(false)) {}

		NativeType* GetType() const
		{
			return static_cast<NativeType*>(initializedType);
		}

	protected:
		Type* Build() override
		{
			NativeType& newType = ReflectionRegistry::Get().AddType<NativeType>(GetId());

			newType.size = sizeof(T);
			newType.id   = id;
			newType.name = name;
			return &newType;
		}
	};
}    // namespace Pipe::Refl
