// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Guid.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"


P_REFLECT_NATIVE_TYPE(bool);
P_REFLECT_NATIVE_TYPE(p::u8);
P_REFLECT_NATIVE_TYPE(p::i8)
P_REFLECT_NATIVE_TYPE(p::u16)
P_REFLECT_NATIVE_TYPE(p::i16)
P_REFLECT_NATIVE_TYPE(p::i32);
P_REFLECT_NATIVE_TYPE(p::u32);
P_REFLECT_NATIVE_TYPE(p::i64);
P_REFLECT_NATIVE_TYPE(p::u64);
P_REFLECT_NATIVE_TYPE(float);
P_REFLECT_NATIVE_TYPE(double);

P_REFLECT_NATIVE_TYPE(p::StringView);
P_REFLECT_NATIVE_TYPE(p::String);
P_REFLECT_NATIVE_TYPE(p::Tag);
P_REFLECT_NATIVE_TYPE(p::Guid);

P_OVERRIDE_TYPE_NAME(p::u8)
P_OVERRIDE_TYPE_NAME(p::i8)
P_OVERRIDE_TYPE_NAME(p::u16)
P_OVERRIDE_TYPE_NAME(p::i16)
P_OVERRIDE_TYPE_NAME(p::u32)
P_OVERRIDE_TYPE_NAME(p::i32)
P_OVERRIDE_TYPE_NAME(p::u64)
P_OVERRIDE_TYPE_NAME(p::i64)
P_OVERRIDE_TYPE_NAME(p::TChar)
P_OVERRIDE_TYPE_NAME(p::StringView)
P_OVERRIDE_TYPE_NAME(p::String)
P_OVERRIDE_TYPE_NAME(p::Tag)
