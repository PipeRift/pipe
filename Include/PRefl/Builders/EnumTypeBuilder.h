// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "PCore/Function.h"
#include "PCore/Name.h"
#include "PCore/TypeTraits.h"
#include "PRefl/Builders/StaticInitializers.h"
#include "PRefl/Builders/TypeBuilder.h"
#include "PRefl/EnumType.h"
#include "PRefl/Registry.h"
#include "PRefl/TypeId.h"


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
		    builder.Initialize();                                    \
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
		Type* Build() override
		{
			EnumType& newType = ReflectionRegistry::Get().AddType<EnumType>(GetId());

			newType.size = sizeof(T);
			newType.id   = id;
			newType.name = name;

			const auto entries = magic_enum::enum_entries<T>();

			newType.valueSize = sizeof(T);
			newType.values.Resize(entries.size() * newType.valueSize);
			for (u32 i = 0; i < entries.size(); ++i)
			{
				// Copy value into the correct entry index
				memcpy(newType.GetValuePtrByIndex(i), &entries[i].first, newType.valueSize);
			}

			newType.names.Reserve(entries.size());
			for (const auto& entry : entries)
			{
				newType.names.Add(Name{entry.second});
			}
			return &newType;
		}
	};
}    // namespace p
