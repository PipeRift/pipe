// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/Type.h"

#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/DataType.h"
#include "Pipe/Reflect/EnumType.h"
#include "Pipe/Reflect/NativeType.h"
#include "Pipe/Reflect/StructType.h"


namespace p
{
	// using namespace EnumOperators;

	StringView Type::GetName() const
	{
		return name;
	}

	NativeType* Type::AsNative()
	{
		return HasFlag(category, TypeCategory::Native) ? static_cast<NativeType*>(this) : nullptr;
	}
	EnumType* Type::AsEnum()
	{
		return HasFlag(category, TypeCategory::Enum) ? static_cast<EnumType*>(this) : nullptr;
	}
	DataType* Type::AsData()
	{
		return HasFlag(category, TypeCategory::Data) ? static_cast<DataType*>(this) : nullptr;
	}
	StructType* Type::AsStruct()
	{
		return HasFlag(category, TypeCategory::Struct) ? static_cast<StructType*>(this) : nullptr;
	}
	ClassType* Type::AsClass()
	{
		return HasFlag(category, TypeCategory::Class) ? static_cast<ClassType*>(this) : nullptr;
	}
}    // namespace p