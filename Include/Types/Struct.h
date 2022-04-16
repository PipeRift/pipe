// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Export.h"
#include "Reflection/Reflection.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	struct Struct : public BaseStruct
	{
	public:
		using Super       = BaseStruct;
		using BuilderType = Rift::Refl::TStructTypeBuilder<Struct, void, Type_NoFlag>;

		static Rift::Refl::StructType* GetStaticType()
		{
			return Rift::GetType<Struct>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		REFLECTION_BODY({})
	};


	template<typename T>
	void Read(Serl::ReadContext& ct, T& value) requires(Derived<T, Struct>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		value.SerializeReflection(common);
	}

	template<typename T>
	void Write(Serl::WriteContext& ct, const T& value) requires(Derived<T, Struct>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		const_cast<T&>(value).SerializeReflection(common);
	}
}    // namespace Rift
