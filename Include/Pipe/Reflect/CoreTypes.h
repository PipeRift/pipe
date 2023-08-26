// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Guid.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"


REFLECT_NATIVE_TYPE(bool);
REFLECT_NATIVE_TYPE(p::u8);
REFLECT_NATIVE_TYPE(p::i8)
REFLECT_NATIVE_TYPE(p::u16)
REFLECT_NATIVE_TYPE(p::i16)
REFLECT_NATIVE_TYPE(p::i32);
REFLECT_NATIVE_TYPE(p::u32);
REFLECT_NATIVE_TYPE(p::i64);
REFLECT_NATIVE_TYPE(p::u64);
REFLECT_NATIVE_TYPE(float);
REFLECT_NATIVE_TYPE(double);

REFLECT_NATIVE_TYPE(p::StringView);
REFLECT_NATIVE_TYPE(p::String);
REFLECT_NATIVE_TYPE(p::Tag);
REFLECT_NATIVE_TYPE(p::Guid);

OVERRIDE_TYPE_NAME(p::u8)
OVERRIDE_TYPE_NAME(p::i8)
OVERRIDE_TYPE_NAME(p::u16)
OVERRIDE_TYPE_NAME(p::i16)
OVERRIDE_TYPE_NAME(p::u32)
OVERRIDE_TYPE_NAME(p::i32)
OVERRIDE_TYPE_NAME(p::u64)
OVERRIDE_TYPE_NAME(p::i64)
OVERRIDE_TYPE_NAME(p::TChar)
OVERRIDE_TYPE_NAME(p::StringView)
OVERRIDE_TYPE_NAME(p::String)
OVERRIDE_TYPE_NAME(p::Tag)
