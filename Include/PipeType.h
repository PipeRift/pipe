// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/Hash.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#include "PipePlatform.h"

#include <format>
#include <iostream>


namespace p
{
	struct P_API TypeId
	{
	protected:
		u64 id;
#if P_DEBUG
		StringView debugName;
#endif


	public:
		constexpr TypeId() : id{0} {}
		constexpr TypeId(p::Undefined) {}
		explicit constexpr TypeId(u64 id) : id{id} {}
#if P_DEBUG
		explicit constexpr TypeId(u64 id, StringView debugName) : id{id}, debugName{debugName} {}
#endif

		constexpr u64 GetId() const
		{
			return id;
		}

		StringView GetDebugName() const
		{
#if P_DEBUG
			return debugName;
#else
			return {};
#endif
		}

		constexpr bool IsValid() const
		{
			return id != 0;
		}

		constexpr auto operator==(const TypeId& other) const
		{
			return id == other.id;
		}
		constexpr auto operator<(const TypeId& other) const
		{
			return id < other.id;
		}
		constexpr auto operator>(const TypeId& other) const
		{
			return id > other.id;
		}
		constexpr auto operator<=(const TypeId& other) const
		{
			return id <= other.id;
		}
		constexpr auto operator>=(const TypeId& other) const
		{
			return id >= other.id;
		}
		constexpr operator bool() const
		{
			return IsValid();
		}

		static consteval TypeId None()
		{
			return TypeId{};
		}
	};

	inline sizet GetHash(const TypeId& id)
	{
		return GetHash(id.GetId());
	}

	inline std::ostream& operator<<(std::ostream& stream, TypeId typeId)
	{
		stream << "TypeId(id=" << typeId.GetId() << ")";
		return stream;
	}


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
				else if (*c == '<')
				{    // Stop on templates
					break;
				}
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
				else if (*c == '<')
				{    // Stop on templates
					break;
				}
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
	inline consteval StringView GetTypeName(bool includeNamespaces = true) requires(IsMap<T>())
	{
		return "TMap";
	}


	template<typename T>
	inline consteval TypeId GetTypeId() requires(!IsConst<T>)
	{
		return TypeId{p::GetStringHash(P_UNIQUE_FUNCTION_ID),
#if P_DEBUG
		    GetTypeName<T>()
#endif
		};
	}

	template<typename T>
	inline consteval TypeId GetTypeId() requires(IsConst<T>)
	{
		return GetTypeId<Mut<T>>();
	}

	// A TypeId bound to a base type.
	template<typename T>
	struct TTypeId : public TypeId
	{
		constexpr TTypeId() : TypeId(GetTypeId<T>()) {}
		constexpr TTypeId(p::Undefined) : TypeId(GetTypeId<T>()) {}

		// From another TTypeId bound to a compatible (same or derived) type.
		template<Derived<T, true> T2>
		constexpr TTypeId(const TTypeId<T2>& other) : TypeId(other)
		{}

		// From a runtime TypeId.
		TTypeId(TypeId id) : TypeId(IsCompatible(GetTypeId<T>(), id) ? id : TypeId{}) {}

		constexpr TTypeId& operator=(const TTypeId&) = default;
		template<Derived<T, true> T2>
		constexpr TTypeId& operator=(const TTypeId<T2>& other)
		{
			TypeId::operator=(other);
			return *this;
		}
		TTypeId& operator=(TypeId id)
		{
			TypeId::operator=(IsCompatible(GetTypeId<T>(), id) ? id : TypeId{});
			return *this;
		}

	private:
		static bool IsCompatible(TypeId parentId, TypeId childId)
		{
			return parentId == childId || IsTypeParentOf(parentId, childId);
		}
	};


#pragma region Castable
	struct Castable
	{
	private:
		mutable TypeId typeId;

	public:
		TypeId GetTypeId() const
		{
			if (!typeId)
			{
				typeId = ProvideTypeId();
			}
			return typeId;
		}

	protected:
		virtual TypeId ProvideTypeId() const = 0;
	};

	template<typename T>
	concept IsCastable = Derived<std::remove_pointer_t<T>, Castable, false>;
#pragma endregion Castable
}    // namespace p


template<>
struct std::formatter<p::TypeId> : public std::formatter<p::u64>
{
	template<typename FormatContext>
	auto format(const p::TypeId& typeId, FormatContext& ctx) const
	{
#if P_DEBUG
		const p::StringView debugName = typeId.GetDebugName();
		if (!debugName.empty())
		{
			return std::formatter<p::StringView>{}.format(debugName, ctx);
		}
#endif
		return formatter<p::u64>::format(typeId.GetId(), ctx);
	}
};

template<typename T>
struct std::formatter<p::TTypeId<T>> : public std::formatter<p::TypeId>
{};

#define P_OVERRIDE_TYPE_NAME(type, name)                                            \
	template<>                                                                      \
	inline consteval p::StringView p::GetFullTypeName<type>(bool includeNamespaces) \
	{                                                                               \
		return name;                                                                \
	}