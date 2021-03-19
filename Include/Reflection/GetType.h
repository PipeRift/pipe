// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/HasType.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Registry/TypeInstance.h"


namespace Rift
{
	namespace Refl
	{
		class StructType;
		class ClassType;
		class EnumType;
	}    // namespace Refl


	template <typename T>
	Refl::StructType* GetType() requires(IsStruct<T>())
	{
		return static_cast<Refl::StructType*>(Refl::InternalGetType<T>());
	}

	template <typename T>
	Refl::ClassType* GetType() requires(IsClass<T>())
	{
		return static_cast<Refl::ClassType*>(Refl::InternalGetType<T>());
	}

	template <typename T>
	Refl::EnumType* GetType() requires(IsEnum<T>())
	{
		return static_cast<Refl::EnumType*>(Refl::InternalGetType<T>());
	}

	template <typename T>
	Refl::Type* GetType()
	{
		return Refl::InternalGetType<T>();
	}
}    // namespace Rift
