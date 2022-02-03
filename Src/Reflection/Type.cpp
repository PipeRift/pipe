// Copyright 2015-2022 Piperift - All rights reserved

#include "Reflection/Type.h"

#include "Reflection/ClassType.h"
#include "Reflection/DataType.h"
#include "Reflection/EnumType.h"
#include "Reflection/NativeType.h"
#include "Reflection/StructType.h"


namespace Rift::Refl
{
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
}    // namespace Rift::Refl
