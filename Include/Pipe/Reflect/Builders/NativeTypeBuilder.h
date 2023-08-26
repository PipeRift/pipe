// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/Builders/TypeBuilder.h"
#include "Pipe/Reflect/NativeType.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeName.h"
#include "Pipe/Reflect/TypeRegistry.h"


#define P_REFLECT_NATIVE_TYPE(type)                                                          \
	template<>                                                                               \
	struct p::TStaticNativeInitializer<type>                                                 \
	{                                                                                        \
		static constexpr bool enabled = true;                                                \
		static const p::TFunction<p::Type*()> onInit;                                        \
	};                                                                                       \
	inline const p::TFunction<p::Type*()> p::TStaticNativeInitializer<type>::onInit = []() { \
		p::TNativeTypeBuilder<type> builder{};                                               \
		builder.BeginBuild();                                                                \
		return builder.GetType();                                                            \
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

	protected:
		Type* CreateType() override
		{
			NativeType& newType = TypeRegistry::Get().AddType<NativeType>(GetId());

			newType.size = sizeof(T);
			newType.id   = id;
			newType.name = name;
			return &newType;
		}
	};
}    // namespace p
