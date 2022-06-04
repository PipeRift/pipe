// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Name.h"
#include "Core/Platform.h"
#include "Core/String.h"
#include "Reflection/Builders/NativeTypeBuilder.h"



namespace Pipe
{
	REFLECT_NATIVE_TYPE(bool);
	REFLECT_NATIVE_TYPE(u8);
	REFLECT_NATIVE_TYPE(i32);
	REFLECT_NATIVE_TYPE(u32);
	REFLECT_NATIVE_TYPE(i64);
	REFLECT_NATIVE_TYPE(u64);
	REFLECT_NATIVE_TYPE(float);
	REFLECT_NATIVE_TYPE(double);

	REFLECT_NATIVE_TYPE(String);
	REFLECT_NATIVE_TYPE(Name);
}    // namespace Pipe
