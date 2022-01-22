// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/Registry/NativeTypeBuilder.h"
#include "Strings/Name.h"
#include "Strings/String.h"


namespace Rift
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
}    // namespace Rift
