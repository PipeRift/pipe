// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Strings/StringView.h"


namespace Rift
{
	namespace Refl::TypeName
	{
		template <class T>
		constexpr StringView GetRaw()
		{
#if defined(_MSC_VER)
			return __FUNCSIG__;
#else
			return __PRETTY_FUNCTION__;
#endif
		}

		constexpr StringView testedRawName = GetRaw<double>();
		constexpr sizet testNameLength     = StringView("double").size();

		constexpr sizet prefixLength = testedRawName.find("double");
		constexpr sizet suffixLength = testedRawName.size() - prefixLength - testNameLength;
		static_assert(
		    prefixLength != StringView::npos, "Can't extract typename from function signature");
	}    // namespace Refl::TypeName


	template <typename T>
	inline constexpr StringView GetTypeName()
	{
		const StringView raw = Refl::TypeName::GetRaw<T>();
		return {raw.data() + Refl::TypeName::prefixLength,
		    raw.size() - Refl::TypeName::prefixLength - Refl::TypeName::suffixLength};
	}


#define OVERRIDE_TYPE_NAME(type)                    \
	template <>                                     \
	inline constexpr StringView GetTypeName<type>() \
	{                                               \
		return TX(#type);                           \
	}

	OVERRIDE_TYPE_NAME(u8)
	OVERRIDE_TYPE_NAME(u16)
	OVERRIDE_TYPE_NAME(u32)
	OVERRIDE_TYPE_NAME(u64)
	OVERRIDE_TYPE_NAME(i8)
	OVERRIDE_TYPE_NAME(i16)
	OVERRIDE_TYPE_NAME(i32)
	OVERRIDE_TYPE_NAME(i64)
	OVERRIDE_TYPE_NAME(TCHAR)
	OVERRIDE_TYPE_NAME(StringView)
}    // namespace Rift
