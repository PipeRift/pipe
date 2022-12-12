// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/Type.h"

#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/DataType.h"
#include "Pipe/Reflect/EnumType.h"
#include "Pipe/Reflect/NativeType.h"
#include "Pipe/Reflect/StructType.h"


namespace p
{
	StringView Type::GetName() const
	{
		return name;
	}
}    // namespace p
