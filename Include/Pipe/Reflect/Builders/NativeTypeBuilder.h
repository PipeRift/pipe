// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Name.h"
#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/Builders/TypeBuilder.h"
#include "Pipe/Reflect/NativeType.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeRegistry.h"


#define REFLECT_NATIVE_TYPE(type)                                      \
	template<>                                                         \
	struct p::reflection::TStaticNativeInitializer<type>               \
	{                                                                  \
		static constexpr bool enabled = true;                          \
		static const p::TFunction<p::NativeType*()> onInit;            \
	};                                                                 \
	inline const p::TFunction<p::NativeType*()>                        \
	    p::reflection::TStaticNativeInitializer<type>::onInit = []() { \
		    p::TNativeTypeBuilder<type> builder{};                     \
		    builder.Initialize();                                      \
		    return builder.GetType();                                  \
	    };


namespace p
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
			NativeType& newType = TypeRegistry::Get().AddType<NativeType>(GetId());

			newType.size = sizeof(T);
			newType.id   = id;
			newType.name = name;
			return &newType;
		}
	};
}    // namespace p
