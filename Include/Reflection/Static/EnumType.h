// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"
#include "Strings/Name.h"

#include <magic_enum.hpp>


namespace Rift::Refl
{
	template<typename T>
	constexpr sizet GetEnumSize()
	{
		return magic_enum::enum_count<T>();
	}

	template<typename T>
	constexpr StringView GetEnumName(T value)
	{
		return magic_enum::enum_name(value);
	}

	template<typename T>
	constexpr std::optional<T> GetEnumValue(StringView str)
	{
		return magic_enum::enum_cast<T>(str);
	}


	/** Smallest reflection type that contains all basic class or struct data */
	class EnumType : public Type
	{
		template<typename T>
		friend struct TEnumTypeBuilder;

	protected:
		Name name;


	public:
		CORE_API EnumType() = default;
	};
}    // namespace Rift::Refl
