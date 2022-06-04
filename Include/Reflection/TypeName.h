// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Strings/StringView.h"


namespace Pipe
{
	namespace Refl::TypeName
	{
		template<class T>
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


	inline constexpr StringView RemoveNamespace(StringView value)
	{
		const sizet pos = Strings::Find(value, "::", FindDirection::Back);
		if (pos != StringView::npos)
		{
			return Strings::RemoveFromStart(value, pos + 2);
		}
		return value;
	}

	template<typename T>
	inline consteval StringView GetFullTypeName(bool includeNamespaces = true)
	{
		const StringView raw = Refl::TypeName::GetRaw<T>();
		StringView typeName{raw.data() + Refl::TypeName::prefixLength,
		    raw.size() - Refl::TypeName::prefixLength - Refl::TypeName::suffixLength};

		typeName = Strings::RemoveFromStart(typeName, "struct ");
		typeName = Strings::RemoveFromStart(typeName, "class ");

		if (!includeNamespaces)
		{
			return RemoveNamespace(typeName);
		}
		return typeName;
	}

	template<typename T>
	inline consteval StringView GetTypeName(bool includeNamespaces = true)
	{
		return GetFullTypeName<T>(includeNamespaces);
	}


#define OVERRIDE_TYPE_NAME(type)                                              \
	template<>                                                                \
	inline consteval StringView GetFullTypeName<type>(bool includeNamespaces) \
	{                                                                         \
		return TX(#type);                                                     \
	}

}    // namespace Pipe
