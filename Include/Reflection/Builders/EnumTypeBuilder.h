// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Reflection/Builders/StaticInitializers.h"
#include "Reflection/Builders/TypeBuilder.h"
#include "Reflection/EnumType.h"
#include "Reflection/Registry.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


#define ENUM(type)                                                    \
	template<>                                                        \
	struct Pipe::Refl::TStaticEnumInitializer<type>                   \
	{                                                                 \
		static constexpr bool enabled = true;                         \
		static const Pipe::TFunction<Pipe::Refl::EnumType*()> onInit; \
	};                                                                \
	inline const Pipe::TFunction<Pipe::Refl::EnumType*()>             \
	    Pipe::Refl::TStaticEnumInitializer<type>::onInit = []() {     \
		    Pipe::Refl::TEnumTypeBuilder<type> builder{};             \
		    builder.Initialize();                                     \
		    return builder.GetType();                                 \
	    };


namespace Pipe::Refl
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
}    // namespace Pipe::Refl
