// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Export.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionTraits.h"
#include "Serialization/Contexts.h"


namespace pipe::refl
{
	struct Struct : public BaseStruct
	{
	public:
		using Super       = BaseStruct;
		using BuilderType = pipe::refl::TStructTypeBuilder<Struct, void, Type_NoFlag>;

		static pipe::refl::StructType* GetStaticType()
		{
			return pipe::GetType<Struct>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		REFLECTION_BODY({})
	};
}    // namespace pipe::refl

namespace pipe
{
	using namespace pipe::refl;

	namespace Serl
	{
		template<typename T>
		void Read(pipe::ReadContext& ct, T& value) requires(pipe::IsStruct<T>())
		{
			ct.BeginObject();
			pipe::CommonContext common{ct};
			value.SerializeReflection(common);
		}

		template<typename T>
		void Write(pipe::WriteContext& ct, const T& value) requires(pipe::IsStruct<T>())
		{
			ct.BeginObject();
			pipe::CommonContext common{ct};
			const_cast<T&>(value).SerializeReflection(common);
		}
	}    // namespace Serl
}    // namespace pipe
