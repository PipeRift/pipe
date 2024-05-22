// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/StringView.h"


namespace p
{
	namespace TypeName
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
	}    // namespace TypeName


	inline constexpr StringView RemoveNamespace(StringView value)
	{
		if (value.size() > 0)
		{
			const char* c               = value.data();
			const char* const end       = c + value.size();
			const char* const last      = end - 1;
			const char* lastFoundQuotes = nullptr;
			while (c < last)    // skip last char
			{
				if (*c == ':' && *(c + 1) == ':')
				{
					lastFoundQuotes = c;
					++c;    // Skip one more character for ::
				}
				else if (*c == '<')    // Stop on templates
					break;
				++c;
			}
			if (lastFoundQuotes)
			{
				return {lastFoundQuotes + 2, end};
			}
		}
		return value;
	}
	inline constexpr StringView RemoveNamespace(StringView value, StringView& outNamespace)
	{
		if (value.size() > 0)
		{
			const char* c               = value.data();
			const char* const end       = c + value.size();
			const char* const last      = end - 1;
			const char* lastFoundQuotes = nullptr;
			while (c < last)    // skip last char
			{
				if (*c == ':' && *(c + 1) == ':')
				{
					lastFoundQuotes = c;
					++c;    // Skip one more character for ::
				}
				else if (*c == '<')    // Stop on templates
					break;
				++c;
			}
			if (lastFoundQuotes)
			{
				outNamespace = {value.data(), lastFoundQuotes};
				return {lastFoundQuotes + 2, end};
			}
		}
		outNamespace = {};
		return value;
	}

	template<typename T>
	inline consteval StringView GetFullTypeName(bool includeNamespaces = true)
	{
		const StringView raw = TypeName::GetRaw<T>();
		StringView typeName{raw.data() + TypeName::prefixLength,
		    raw.size() - TypeName::prefixLength - TypeName::suffixLength};

		typeName = Strings::RemoveFromStart(typeName, "struct ");
		typeName = Strings::RemoveFromStart(typeName, "class ");
		typeName = Strings::RemoveFromStart(typeName, "enum ");

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


	namespace TypeName
	{
		// Contains an static fixed string with the name of a TArray<T>
		template<typename ItemType, bool includeNamespaces>
		struct Array
		{
			static constexpr auto preffix        = TFixedString("TArray<");
			static constexpr auto suffix         = TFixedString(">");
			static constexpr StringView itemName = GetFullTypeName<ItemType>(includeNamespaces);
			static constexpr TFixedString<itemName.size()> fixedItemName{itemName};

			static constexpr auto name = preffix + fixedItemName + suffix;
		};

		// Contains an static fixed string with the name of a TMap<Key, Value>
		template<typename KeyType, typename ValueType, bool includeNamespaces>
		struct Map
		{
			static constexpr auto preffix         = TFixedString("TMap<");
			static constexpr auto separator       = TFixedString(", ");
			static constexpr auto suffix          = TFixedString(">");
			static constexpr StringView keyName   = GetFullTypeName<KeyType>(includeNamespaces);
			static constexpr StringView valueName = GetFullTypeName<ValueType>(includeNamespaces);
			static constexpr TFixedString<keyName.size()> fixedKeyName{keyName};
			static constexpr TFixedString<valueName.size()> fixedValueName{valueName};

			static constexpr auto name =
			    preffix + fixedKeyName + separator + fixedValueName + suffix;
		};
	}    // namespace TypeName


	template<typename T>
	consteval StringView GetFullTypeName(bool includeNamespaces = true) requires(IsArray<T>())
	{
		if (includeNamespaces)
		{
			return TypeName::Array<typename T::ItemType, true>::name;
		}
		return TypeName::Array<typename T::ItemType, false>::name;
	}

	template<typename T>
	inline consteval StringView GetTypeName(bool includeNamespaces = true) requires(IsArray<T>())
	{
		return "TArray";
	}

	template<typename T>
	consteval StringView GetFullTypeName(bool includeNamespaces = true) requires(IsMap<T>())
	{
		if (includeNamespaces)
		{
			return TypeName::Map<typename T::KeyType, typename T::ValueType, true>::name;
		}
		return TypeName::Map<typename T::KeyType, typename T::ValueType, false>::name;
	}

	template<typename T>
	consteval StringView GetTypeName(bool includeNamespaces = true) requires(IsMap<T>())
	{
		return "TMap";
	}
}    // namespace p

#define P_OVERRIDE_TYPE_NAME(type, name)                                            \
	template<>                                                                      \
	inline consteval p::StringView p::GetFullTypeName<type>(bool includeNamespaces) \
	{                                                                               \
		return TX(name);                                                            \
	}
