// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "Reflection/Reflection.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	struct CORE_API Struct : public BaseStruct
	{
		ORPHAN_STRUCT(Struct, ReflectionTags::None)
	};


	template <typename T>
	void Read(Serl::ReadContext& ct, T& value) requires(Derived<T, Struct>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		value.SerializeReflection(common);
	}

	template <typename T>
	void Write(Serl::WriteContext& ct, const T& value) requires(Derived<T, Struct>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		const_cast<T&>(value).SerializeReflection(common);
	}
}    // namespace Rift
