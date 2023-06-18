// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Pipe/Reflect/Reflection.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Serialize/Serialization.h"


namespace p
{
	struct Struct : public BaseStruct
	{
	public:
		using Super       = BaseStruct;
		using BuilderType = p::TStructTypeBuilder<Struct, void, Type_NoFlag>;

		static p::StructType* GetStaticType()
		{
			return p::GetType<Struct>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		REFLECTION_BODY({})
	};


	template<typename T>
	void Read(Reader& ct, T& value)
	    requires(IsStruct<T>() && !TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize)
	{
		ct.BeginObject();
		ReadWriter common{ct};
		value.SerializeReflection(common);
	}

	template<typename T>
	void Write(Writer& ct, const T& value)
	    requires(IsStruct<T>() && !TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize)
	{
		ct.BeginObject();
		ReadWriter common{ct};
		const_cast<T&>(value).SerializeReflection(common);
	}
}    // namespace p
