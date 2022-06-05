// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Export.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionTraits.h"
#include "Serialization/Contexts.h"


namespace p::refl
{
	struct Struct : public BaseStruct
	{
	public:
		using Super       = BaseStruct;
		using BuilderType = p::refl::TStructTypeBuilder<Struct, void, Type_NoFlag>;

		static p::refl::StructType* GetStaticType()
		{
			return p::GetType<Struct>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		REFLECTION_BODY({})
	};
}    // namespace p::refl

namespace p
{
	using namespace p::refl;

	namespace serl
	{
		template<typename T>
		void Read(p::ReadContext& ct, T& value) requires(p::IsStruct<T>())
		{
			ct.BeginObject();
			p::CommonContext common{ct};
			value.SerializeReflection(common);
		}

		template<typename T>
		void Write(p::WriteContext& ct, const T& value) requires(p::IsStruct<T>())
		{
			ct.BeginObject();
			p::CommonContext common{ct};
			const_cast<T&>(value).SerializeReflection(common);
		}
	}    // namespace serl
}    // namespace p
