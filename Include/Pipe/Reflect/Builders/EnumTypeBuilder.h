// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"
#include "Pipe/Core/Name.h"
#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/Builders/TypeBuilder.h"
#include "Pipe/Reflect/EnumType.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeRegistry.h"


#define ENUM(type)                                                   \
	template<>                                                       \
	struct p::reflection::TStaticEnumInitializer<type>               \
	{                                                                \
		static constexpr bool enabled = true;                        \
		static const p::TFunction<p::EnumType*()> onInit;            \
	};                                                               \
	inline const p::TFunction<p::EnumType*()>                        \
	    p::reflection::TStaticEnumInitializer<type>::onInit = []() { \
		    p::TEnumTypeBuilder<type> builder{};                     \
		    builder.BeginBuild();                                    \
		    return builder.GetType();                                \
	    };


namespace p
{
	/**
	 * Enum Type Builder
	 * Builds enum types during static initialization
	 */
	template<typename T>
	struct TEnumTypeBuilder : public TypeBuilder
	{
	public:
		TEnumTypeBuilder() : TypeBuilder(GetTypeId<T>(), GetTypeName<T>(false)) {}

		EnumType* GetType() const
		{
			return static_cast<EnumType*>(initializedType);
		}

	protected:
		Type* CreateType() override
		{
			EnumType& newType = TypeRegistry::Get().AddType<EnumType>(GetId());

			newType.size = sizeof(T);
			newType.id   = id;
			newType.name = name;

			const auto names = GetEnumNames<T>();
			newType.names.Reserve(names.size());
			for (StringView name : names)
			{
				newType.names.Add(Name{name});
			}

			const auto values = GetEnumValues<T>();
			newType.valueSize = sizeof(T);
			newType.values.Resize(values.size() * newType.valueSize);
			for (u32 i = 0; i < values.size(); ++i)
			{
				// Copy value into the correct entry index
				memcpy(newType.GetValuePtrByIndex(i), &values[i], newType.valueSize);
			}
			return &newType;
		}
	};
}    // namespace p
