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
	inline constexpr StringView GetFullTypeName()
	{
		const StringView raw = Refl::TypeName::GetRaw<T>();
		StringView typeName{raw.data() + Refl::TypeName::prefixLength,
		    raw.size() - Refl::TypeName::prefixLength - Refl::TypeName::suffixLength};

		typeName = Strings::RemoveFromStart(typeName, "struct ");
		typeName = Strings::RemoveFromStart(typeName, "class ");
		return typeName;
	}

	template <typename T>
	inline constexpr StringView GetTypeName()
	{
		return GetFullTypeName<T>();
	}


#define OVERRIDE_TYPE_NAME(type)                        \
	template <>                                         \
	inline constexpr StringView GetFullTypeName<type>() \
	{                                                   \
		return TX(#type);                               \
	}

}    // namespace Rift
