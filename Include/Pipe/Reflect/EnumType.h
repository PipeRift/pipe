// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Name.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/Type.h"

#include <magic_enum.hpp>


namespace p
{
	template<typename T>
	constexpr sizet GetEnumSize()
	{
		return magic_enum::enum_count<T>();
	}

	template<typename T>
	constexpr std::optional<T> GetEnumValue(StringView str)
	{
		return magic_enum::enum_cast<T>(str);
	}

	template<typename T>
	constexpr auto GetEnumValues()
	{
		return magic_enum::enum_values<T>();
	}

	template<typename T>
	constexpr StringView GetEnumName(T value)
	{
		return magic_enum::enum_name(value);
	}

	template<typename T>
	constexpr auto GetEnumNames()
	{
		return magic_enum::enum_names<T>();
	}

	template<typename T>
	constexpr void GetEnumFlagName(T value, String& outName)
	{
		bool hasAny = false;
		for (auto v : GetEnumValues<T>())
		{
			if (static_cast<UnderlyingType<T>>(v) != 0 && HasFlag(value, v))
			{
				Strings::FormatTo(outName, "{} | ", GetEnumName(v));
				hasAny = true;
			}
		}
		if (hasAny)
		{
			Strings::RemoveFromEnd(outName, 3);
		}
	}


	/** Smallest reflection type that contains all basic class or struct data */
	class EnumType : public Type
	{
		template<typename T>
		friend struct TEnumTypeBuilder;

	protected:
		u32 valueSize = 0;
		// Values contains all values sequentially (according to valueSize)
		TArray<u8> values;
		TArray<Name> names;

	public:
		static constexpr TypeCategory typeCategory = TypeCategory::Enum;


	public:
		PIPE_API EnumType() : Type(typeCategory) {}


		template<Integral T>
		void SetValue(void* data, T value) const
		{
			Check(sizeof(T) >= valueSize);
			memcpy(data, &data, valueSize);
		}
		void SetValue(void* data, Name name) const
		{
			i32 index = names.FindIndex(name);
			if (index != NO_INDEX)
			{
				SetValueFromIndex(data, index);
			}
		}

		template<Integral T>
		T GetValue(void* data) const
		{
			Check(sizeof(T) >= valueSize);
			return *reinterpret_cast<T*>(data);
		}

		Name GetName(void* data) const
		{
			const i32 index = GetIndexFromValue(data);
			return index != NO_INDEX ? GetNameByIndex(index) : Name::None();
		}

		i32 GetIndexFromValue(void* data) const
		{
			for (i32 i = 0; i < Size(); ++i)
			{
				const void* valuePtr = GetValuePtrByIndex(i);
				if (memcmp(data, valuePtr, valueSize) == 0)
				{
					// Value matches
					return i;
				}
			}
			return NO_INDEX;
		}
		void SetValueFromIndex(void* data, i32 index) const
		{
			const void* valuePtr = GetValuePtrByIndex(index);
			memcpy(data, valuePtr, valueSize);
		}

		template<Integral T>
		const T& GetValueByIndex(i32 index) const
		{
			return *reinterpret_cast<T*>(GetValuePtrByIndex(index));
		}
		Name GetNameByIndex(i32 index) const
		{
			return names.IsValidIndex(index) ? names[index] : Name::None();
		}

		// Intentionally unsafe function. Do not modify the value!
		void* GetValuePtrByIndex(i32 index)
		{
			return (void*)(values.Data() + (index * valueSize));
		}
		const void* GetValuePtrByIndex(i32 index) const
		{
			return (void*)(values.Data() + (index * valueSize));
		}

		i32 Size() const
		{
			return names.Size();
		}
	};
}    // namespace p
