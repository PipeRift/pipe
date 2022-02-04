// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Misc/EnumFlags.h"
#include "Reflection/TypeId.h"
#include "Strings/StringView.h"


namespace Rift::Refl
{
	using namespace EnumOperators;

	enum class TypeCategory : u8
	{
		None   = 1 << 0,
		Native = 1 << 1,
		Enum   = 1 << 2,
		Data   = 1 << 3,
		Struct = 1 << 4,
		Class  = 1 << 5,
	};

	/** Smallest reflection type */
	class CORE_API Type
	{
	protected:
		TypeId id;
		TypeCategory category = TypeCategory::None;
		StringView name;


	protected:
		Type(TypeCategory category) : category{category} {}

	public:
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;

		TypeId GetId() const
		{
			return id;
		}
		StringView GetName() const;

		class NativeType* AsNative();
		class EnumType* AsEnum();
		class DataType* AsData();
		class StructType* AsStruct();
		class ClassType* AsClass();
	};
}    // namespace Rift::Refl
