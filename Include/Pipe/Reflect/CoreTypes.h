// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Name.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"


namespace p
{
	REFLECT_NATIVE_TYPE(bool);
	REFLECT_NATIVE_TYPE(u8);
	REFLECT_NATIVE_TYPE(i8)
	REFLECT_NATIVE_TYPE(u16)
	REFLECT_NATIVE_TYPE(i16)
	REFLECT_NATIVE_TYPE(i32);
	REFLECT_NATIVE_TYPE(u32);
	REFLECT_NATIVE_TYPE(i64);
	REFLECT_NATIVE_TYPE(u64);
	REFLECT_NATIVE_TYPE(float);
	REFLECT_NATIVE_TYPE(double);

	REFLECT_NATIVE_TYPE(StringView);
	REFLECT_NATIVE_TYPE(String);
	REFLECT_NATIVE_TYPE(Name);

	OVERRIDE_TYPE_NAME(u8)
	OVERRIDE_TYPE_NAME(i8)
	OVERRIDE_TYPE_NAME(u16)
	OVERRIDE_TYPE_NAME(i16)
	OVERRIDE_TYPE_NAME(u32)
	OVERRIDE_TYPE_NAME(i32)
	OVERRIDE_TYPE_NAME(u64)
	OVERRIDE_TYPE_NAME(i64)
	OVERRIDE_TYPE_NAME(TChar)
	OVERRIDE_TYPE_NAME(StringView)
}    // namespace p
